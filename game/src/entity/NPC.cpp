
#include <fornani/automa/SceneContext.hpp>
#include <fornani/core/Debug.hpp>
#include <fornani/entities/player/Player.hpp>
#include <fornani/entity/NPC.hpp>
#include <fornani/events/ConsoleEvent.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/world/Map.hpp>

namespace fornani {

constexpr float default_walk_speed_v = 0.8f;

NPC::NPC(automa::ServiceProvider& svc, dj::Json const& in)
	: Entity(svc, in, "npcs"), Mobile(svc, "npc_" + std::string{in["label"].as_string()}, {svc.data.npc[in["label"].as_string()]["sprite_dimensions"][0].as<int>(), svc.data.npc[in["label"].as_string()]["sprite_dimensions"][1].as<int>()}),
	  m_label(in["label"].as_string()), m_indicator(svc, "arrow_indicator", {16, 16}), m_id{svc.data.npc[in["label"].as_string()]["id"].as<int>()}, m_current_conversation{1}, m_services{&svc}, m_disappear{100}, m_indicator_timer{800} {
	unserialize(in);
	repeatable = false;
	copyable = false;
	Mobile::p_animatable.center();
	Mobile::p_animatable.center();
	m_indicator.center();

	NANI_LOG_DEBUG(Entity::m_logger, "Created NPC with label {}", m_label);

	auto scaled_pos = sf::Vector2f{in["position"][0].as<float>(), in["position"][1].as<float>()};
	auto new_pos = scaled_pos * constants::f_cell_size;
	Mobile::p_animatable.set_position(new_pos);
	auto push = true;
	auto fail_tag = std::string{};
	if (in["contingencies"].is_array()) {
		auto cont = QuestContingencySet{in["contingencies"]};
		if (!svc.quest_table.are_contingencies_met({cont})) {
			hide();
			NANI_LOG_DEBUG(Entity::m_logger, "NPC did not meet contingency for quest.");
		}
	}
	auto npc_state = svc.quest_table.get_quest_progression("npc_dialogue", {m_label, in["id"].as<int>()});
	for (auto const& convo : in["suites"][npc_state].as_array()) {
		push_conversation(convo.as<int>());
		NANI_LOG_DEBUG(Entity::m_logger, "Pushed conversation {}", convo.as<int>());
	}
}

NPC::NPC(automa::ServiceProvider& svc, world::Map& map, dj::Json const& in)
	: Entity(svc, in, "npcs"),
	  Mobile(svc, map, "npc_" + std::string{in["label"].as_string()}, {svc.data.npc[in["label"].as_string()]["sprite_dimensions"][0].as<int>(), svc.data.npc[in["label"].as_string()]["sprite_dimensions"][1].as<int>()}),
	  m_label(in["label"].as_string()), m_indicator(svc, "arrow_indicator", {16, 16}), m_id{svc.data.npc[in["label"].as_string()]["id"].as<int>()}, m_current_conversation{1}, m_services{&svc}, m_disappear{100} {
	unserialize(in);
	repeatable = false;
	copyable = false;
	Mobile::p_animatable.center();
	Mobile::p_animatable.center();
	m_indicator.center();

	NANI_LOG_DEBUG(Entity::m_logger, "Created NPC with label {}", m_label);

	init(svc, svc.data.npc[in["label"].as_string()]);
	set_position_from_scaled(sf::Vector2f{in["position"][0].as<float>(), in["position"][1].as<float>()});
	auto push = true;
	auto fail_tag = std::string{};
	if (p_contingencies) {
		if (!svc.quest_table.are_contingencies_met(*p_contingencies)) {
			hide();
			NANI_LOG_DEBUG(Entity::m_logger, "NPC did not meet contingencies for quests.");
		}
	}
	auto npc_state = svc.quest_table.get_quest_progression("npc_dialogue", {m_label, in["id"].as<int>()});
	for (auto const& convo : in["suites"][npc_state].as_array()) {
		push_conversation(convo.as<int>());
		NANI_LOG_DEBUG(Entity::m_logger, "Pushed conversation {}", convo.as<int>());
	}
}

NPC::NPC(automa::ServiceProvider& svc, world::Map& map, std::string_view label, bool include_collider)
	: Entity(svc, "npcs", 0), Mobile(svc, map, "npc_" + std::string{label}, {svc.data.npc[label]["sprite_dimensions"][0].as<int>(), svc.data.npc[label]["sprite_dimensions"][1].as<int>()}, include_collider), m_label(label),
	  m_indicator(svc, "arrow_indicator", {16, 16}), m_id{svc.data.npc[label]["id"].as<int>()}, m_current_conversation{1}, m_services{&svc} {
	init(svc, svc.data.npc[label]);
}

NPC::NPC(automa::ServiceProvider& svc, int id, std::string_view label, std::vector<std::vector<int>> const suites)
	: Entity(svc, "npcs", id, {1, 1}), Mobile(svc, "npc_" + std::string{label}, {svc.data.npc[label]["sprite_dimensions"][0].as<int>(), svc.data.npc[label]["sprite_dimensions"][1].as<int>()}), m_label(label),
	  m_indicator(svc, "arrow_indicator", {16, 16}), m_id{svc.data.npc[label]["id"].as<int>()}, m_current_conversation{1}, m_suites{suites}, m_services{&svc}, m_walk_speed{default_walk_speed_v} {
	repeatable = false;
	copyable = false;
	set_flag(NPCFlags::face_player);
}

void NPC::init(automa::ServiceProvider& svc, dj::Json const& in_data) {
	svc.events.npc_voice_cue_event.attach_to(slot, &NPC::play_voice_cue, this);
	svc.events.npc_pop_conversation_event.attach_to(slot, &NPC::pop_conversation, this);
	svc.events.npc_piggyback_event.attach_to(slot, &NPC::piggyback_me, this);

	m_offset = sf::Vector2f{in_data["sprite_offset"][0].as<float>(), in_data["sprite_offset"][1].as<float>()};
	if (in_data["vendor"] && svc.data.marketplace.contains(get_specifier())) {
		vendor = &svc.data.marketplace.at(get_specifier());
		NANI_LOG_INFO(Entity::m_logger, "This NPC is a vendor: {}", get_tag());
	}
	m_walk_chance = in_data["walk_chance"].as<int>();
	if (m_walk_chance == 0) { m_walk_chance = 20; }
	if (in_data["random_walk"].as_bool()) { set_flag(NPCFlags::random_walk); }
	m_walk_speed = in_data["walk_speed"] ? in_data["walk_speed"].as<float>() : default_walk_speed_v;

	m_busy_timer.set_native_time(1800);

	if (collider.has_value()) {
		get_collider().dimensions = {in_data["dimensions"][0].as<float>(), in_data["dimensions"][1].as<float>()};
		get_collider().sync_components();
		get_collider().physics.set_friction_componentwise({0.95f, 0.995f});
		get_collider().stats.GRAV = 4.2f;
		get_collider().set_trait(shape::ColliderTrait::npc);
		get_collider().set_exclusion_target(shape::ColliderTrait::circle);
		get_collider().set_exclusion_target(shape::ColliderTrait::enemy);
		get_collider().set_exclusion_target(shape::ColliderTrait::player);
		get_collider().set_exclusion_target(shape::ColliderTrait::npc);
		get_collider().set_exclusion_target(shape::ColliderTrait::pushable);
		get_collider().set_exclusion_target(shape::ColliderTrait::prop);
	}

	for (auto const& in_anim : in_data["animation"].as_array()) {
		Mobile::p_animatable.push_animation(in_anim["label"].as_string(),
											{in_anim["parameters"][0].as<int>(), in_anim["parameters"][1].as<int>(), in_anim["parameters"][2].as<int>(), in_anim["parameters"][3].as<int>(), in_anim["parameters"][4].as_bool()});
		if (in_anim["label"].as_string() == "turn") { set_flag(NPCFlags::has_turn_animation); }
		for (auto const& sound : in_anim["sounds"].as_array()) {
			for (auto const& frame : sound["frames"].as_array()) {
				p_sounds.insert({{frame.as<int>(), in_anim["label"].as_string_view()}, MobileSound{sound["tag"].as_string()}});
				NANI_LOG_INFO(Entity::m_logger, "pushed sound {} for animation {} at frame {}", sound["tag"].as_string(), in_anim["label"].as_string(), frame.as<int>());
			}
		}
	}
	for (auto [i, cue] : std::views::enumerate(in_data["voice_cues"].as_array())) { m_voice_cues.insert({static_cast<int>(i), NPCVoiceCue{m_label + "_" + cue["tag"].as_string()}}); }
	if (Mobile::p_animatable.has_animation("idle")) { Mobile::p_animatable.set_animation("idle"); }
	if (in_data["no_animation"].as_bool()) { set_flag(NPCFlags::no_animation); }
	request(NPCAnimationState::idle);

	if (m_hidden) { m_state.set(NPCState::hidden); }
	if (m_background) { set_flag(NPCFlags::background); }

	if (m_start_busy) {
		state_function = {[this]() { return this->update_busy(); }};
		set_flag(NPCFlags::busy);
		request(NPCAnimationState::busy);
		Mobile::p_animatable.set_animation("busy");
	}

	handle_spawning(svc, in_data);
}

void NPC::handle_spawning(automa::ServiceProvider& svc, dj::Json const& in_data) {

	// There are multiple methods to determine if an NPC should spawn in a room. Combining them might be tricky, but shouldn't be necessary.

	// simple percent chance (like Bongo)
	if (m_spawn) {
		if (m_spawn->chance > 0.f) {
			if (m_spawn->chance < svc.world_clock.get_rng(static_cast<WorldClockInterval>(m_spawn->interval))) { hide(); }
		}
	}

	// schedule (simple night/day activities, like Gaia)
	if (in_data["schedule"]) { m_schedule.emplace(in_data["schedule"]); }
	if (m_schedule) {
		if (!m_schedule->is_here(svc.current_room, svc.world_clock.get_time_of_day())) { hide(); }
	}

	// distribution by time interval (complex room selection and distribution, like Gobe)
	if (in_data["locations"]) {
		NANI_LOG_INFO(m_logger, "Spawning NPC {} based on location distribution.", m_label);
		auto tag = in_data["locations"]["contingency"]["tag"].as_string();
		auto status = svc.quest_table.get_quest_progression(tag);
		NANI_LOG_INFO(m_logger, "Using quest {} to target status index of {}.", tag, status);
		auto const& target_data = in_data["locations"]["statuses"][status];
		auto interval = static_cast<WorldClockInterval>(target_data["interval"].as<int>());
		auto chance = svc.world_clock.get_rng(interval);
		auto cumulative = 0.f;
		auto spawn_me = false;
		for (auto const& room : target_data["distributions"].as_array()) {
			cumulative += room["weight"].as<float>();
			if (chance < cumulative) {
				if (room["room"].as<int>() == svc.current_room) {
					spawn_me = true;
					NANI_LOG_INFO(m_logger, "Spawned {} in room {} with chance {} from weight {}.", m_label, svc.current_room, chance, room["weight"].as<float>());
				}
				break;
			}
		}
		if (target_data["schedule"]) { m_schedule.emplace(target_data["schedule"]); }
		if (m_schedule) {
			if (m_schedule->is_here(svc.current_room, svc.world_clock.get_time_of_day())) { spawn_me = true; }
		}
		if (!spawn_me) { hide(); }
	}
}

void NPC::give_prop(automa::ServiceProvider& svc, world::Map& map, std::string_view label, sf::Vector2i dimensions) { m_mobile_prop.emplace(svc, map, label, dimensions); }

void NPC::drop_prop() {
	if (!m_mobile_prop) { return; }
	m_mobile_prop->drop();
}

void NPC::pick_up_prop() {
	if (!m_mobile_prop) { return; }
	m_mobile_prop->set_flag(MobilePropFlags::dropped, false);
}

void NPC::give_vehicle(automa::ServiceProvider& svc, world::Map& map, std::string_view label) {
	m_vehicle.emplace(svc, map, label);
	m_vehicle->set_position(get_collider().get_center());
}

void NPC::serialize(dj::Json& out) {
	Entity::serialize(out);
	out["background"] = m_background;
	out["label"] = m_label;
	out["hidden"] = m_hidden;
	out["direction"] = static_cast<int>(directions.actual.as_float());
	out["face_player"] = has_flag_set(NPCFlags::face_player);
	out["start_busy"] = m_start_busy;
	if (m_spawn) {
		out["spawn"]["chance"] = m_spawn->chance;
		out["spawn"]["interval"] = m_spawn->interval;
	}
	for (auto& suite : m_suites) {
		auto entry = dj::Json::empty_array();
		for (auto& set : suite) { entry.push_back(set); }
		out["suites"].push_back(entry);
	}
}

void NPC::unserialize(dj::Json const& in) {
	Entity::unserialize(in);
	m_background = in["background"].as_bool();
	set_flag(NPCFlags::face_player, in["face_player"].as_bool());
	set_direction(SimpleDirection{in["direction"].as<int>()});
	m_label = in["label"].as_string();
	m_hidden = in["hidden"].as_bool();
	m_start_busy = in["start_busy"].as_bool();
	if (in["spawn"]) { m_spawn.emplace(in["spawn"]); }
	for (auto const& suite : in["suites"].as_array()) {
		auto entry = std::vector<int>{};
		for (auto const& set : suite.as_array()) { entry.push_back(set.as<int>()); }
		m_suites.push_back(entry);
	}
}

void NPC::expose() {
	Entity::expose();
	ImGui::Checkbox("Hidden?", &m_hidden);
	if (p_contingencies) {
		for (auto [i, ct] : std::views::enumerate(p_contingencies->contingencies)) {
			ImGui::PushID(i);
			ImGui::Text("Contingency %i: [%s, %i]", i, ct.tag.c_str(), ct.requirement);
			ImGui::PopID();
		}
	}
}

void NPC::update([[maybe_unused]] automa::ServiceProvider& svc, [[maybe_unused]] world::Map& map, [[maybe_unused]] SceneContext& context, [[maybe_unused]] player::Player& player) {

	if (m_mobile_prop) {
		m_mobile_prop->set_target(m_prop_socket);
		m_mobile_prop->update(map);
	}
	if (m_vehicle) {
		if (has_flag_set(NPCFlags::airborne)) {
			m_vehicle->set_target(get_collider().get_center() + m_offset);
			m_vehicle->set_idle(false);
		} else {
			m_vehicle->set_idle(true);
		}
		m_vehicle->set_desired_direction(directions.actual);
		m_vehicle->update(svc, map, player);
	}

	if (has_flag_set(NPCFlags::face_player) && !has_flag_set(NPCFlags::cutscene)) { face_player(player); }
	svc.data.set_npc_location(m_id.get(), m_current_location);

	if (consume_flag(NPCFlags::piggyback)) { player.piggyback(m_id.get()); }
	if (is_hidden()) { return; }

	m_indicator.tick();
	m_busy_timer.update();
	m_disappear.update();
	if (m_disappear.is_almost_complete()) { hide(); }

	context.console.has_value() && m_state.test(NPCState::overlapping_player) ? m_state.set(NPCState::talking) : m_state.reset(NPCState::talking);

	if (collider.has_value()) {
		get_collider().update(svc);
		get_collider().detect_map_collision(map);
		get_collider().reset();
		get_collider().physics.acceleration = {};
	}
	auto overlap = collider.has_value() ? player.get_collider().bounding_box.overlaps(get_collider().bounding_box) : false;
	overlap ? m_state.set(NPCState::overlapping_player) : m_state.reset(NPCState::overlapping_player);
	if ((overlap && player.controller.inspecting()) || m_state.test(NPCState::force_interact)) {
		if (!conversations.empty() && !player.has_flag_set(player::PlayerFlags::in_front_of_door)) {
			m_state.set(NPCState::talking);
			start_conversation(svc, context.console);
			player.set_busy(true);
			if (is_state(NPCAnimationState::busy)) { request(NPCAnimationState::respond); }
		}
	}
	if (m_state.test(NPCState::overlapping_player) && m_state.test(NPCState::player_walked_away)) {
		m_indicator.set_parameters(anim::Parameters{0, 15, 16, 0, true});
		m_state.reset(NPCState::player_walked_away);
	}

	if (!overlap) { m_state.set(NPCState::player_walked_away); }

	if (!has_flag_set(NPCFlags::custom_camera)) {
		if (m_state.test(NPCState::talking)) {
			svc.camera_controller.free();
		} else if (!has_flag_set(NPCFlags::cutscene) && overlap) {
			svc.camera_controller.constrain();
		}
	}

	if (has_flag_set(NPCFlags::random_walk)) {
		if (svc.ticker.every_second()) {
			if (random::percent_chance(m_walk_chance)) {
				request(NPCAnimationState::walk);
				m_state.set(NPCState::random_walk);
			}
		}
	}
	if (collider.has_value()) {
		if (!get_collider().grounded() && !has_flag_set(NPCFlags::airborne)) { request(NPCAnimationState::fall); }
	}
	if (directions.actual.lnr != directions.desired.lnr && (!has_flag_set(NPCFlags::busy) || m_busy_timer.running()) && !was_requested(NPCAnimationState::special_2)) { request(NPCAnimationState::turn); }

	if (!context.console.has_value()) { m_state.reset(NPCState::talking); }

	if (!has_flag_set(NPCFlags::no_animation)) { state_function = std::move(state_function()); }
	if (is_hidden()) { return; }
	Mobile::post_update(svc, map, player);
}

void NPC::render(sf::RenderWindow& win, sf::Vector2f cam, float size) {
	switch (debug::mode) {
	case debug::PresentationMode::production:
		if (m_editor) {
			highlighted ? drawbox.setFillColor(sf::Color{250, 80, 250, 60}) : drawbox.setFillColor(sf::Color::Transparent);
			Entity::render(win, cam, size);
		} else {
			if (is_hidden() || m_state.test(NPCState::invisible)) { return; }
			if (collider.has_value()) {
				Mobile::p_animatable.set_position(get_collider().get_center() + m_offset - cam);
				auto indicator_offset = sf::Vector2f{0.f, -constants::f_cell_size};
				m_indicator.set_position(get_collider().get_top() + indicator_offset - cam);
			}
			if (m_vehicle && has_flag_set(NPCFlags::background)) {
				m_vehicle->render(win, cam, DrawOrder::back);
				if (!has_flag_set(NPCFlags::in_vehicle)) { m_vehicle->render(win, cam, DrawOrder::front); }
			}
			if (!has_flag_set(NPCFlags::no_animation)) { win.draw(Mobile::p_animatable); }

			if (!has_flag_set(NPCFlags::no_animation) && !has_flag_set(NPCFlags::cutscene)) { win.draw(m_indicator); }
		}
		break;
	case debug::PresentationMode::debug:
		if (collider.has_value()) { get_collider().render(win, cam); }
		break;
	case debug::PresentationMode::greyblock:
		if (collider.has_value()) { get_collider().render(win, cam); }
		break;
	}
	if (m_vehicle && has_flag_set(NPCFlags::in_vehicle) && has_flag_set(NPCFlags::background)) { m_vehicle->render(win, cam, DrawOrder::front); }
	if (m_mobile_prop) { m_mobile_prop->render(win, cam); }
}

void NPC::render_props(sf::RenderWindow& win, sf::Vector2f cam, DrawOrder order) {
	if (has_flag_set(NPCFlags::background)) { return; }
	if (m_vehicle) {
		m_vehicle->render(win, cam, DrawOrder::back);
		m_vehicle->render(win, cam, DrawOrder::front);
	}
}

void NPC::start_conversation(automa::ServiceProvider& svc, std::optional<std::unique_ptr<gui::Console>>& console) {
	std::string name = std::string(m_label);
	std::string target = std::to_string(conversations.at(m_current_conversation.get()));
	if (!svc.text.npc[name][target].is_object()) {
		NANI_LOG_ERROR(Entity::m_logger, "Tried to start a conversation that doesn't exist!");
		return;
	}
	console = std::make_unique<gui::Console>(svc, svc.text.npc[name][target], static_cast<gui::OutputType>(svc.text.npc[name][target]["output"].as<int>()));
	console.value()->include_portrait(m_id.get());
}

void NPC::push_conversation(int convo) {
	conversations.push_back(convo);
	m_current_conversation.set_order(static_cast<int>(conversations.size()));
	// NANI_LOG_DEBUG(m_logger, "Current order Push: {}", static_cast<int>(conversations.size()));
}

void NPC::pop_conversation() {
	if (conversations.size() > 1) { conversations.pop_front(); }
	m_state.reset(NPCState::cutscene); // this function should only be called for cutscenes
}

void NPC::play_voice_cue(automa::ServiceProvider& svc, int which) const {
	if (!m_state.test(NPCState::talking)) { return; }
	if (svc.soundboard.npc_map.contains(m_label)) { svc.soundboard.npc_map.at(m_label)(which); }
	if (auto it = m_voice_cues.find(which); it != m_voice_cues.end()) { svc.soundboard.play_sound(it->second.tag); }
}

void NPC::piggyback_me(automa::ServiceProvider& svc, int id) {
	if (id != m_id.get()) { return; }
	is_hidden() ? unhide() : hide();
	set_flag(NPCFlags::piggyback);
	svc.camera_controller.constrain();
	NANI_LOG_DEBUG(Entity::m_logger, "Started piggybacking NPC {}", id);
	NANI_LOG_DEBUG(Entity::m_logger, "ID from text was {}", id);
	NANI_LOG_DEBUG(Entity::m_logger, "ID from NPC was {}", m_id.get());
}

void NPC::flush_conversations() { conversations.clear(); }

void NPC::flush_and_push(int convo) {
	flush_conversations();
	push_conversation(convo);
}

void NPC::force_engage() {
	m_state.set(NPCState::cutscene);
	m_state.set(NPCState::force_interact);
}

void NPC::disengage() {
	m_state.reset(NPCState::cutscene);
	m_state.reset(NPCState::force_interact);
}

void NPC::set_special_animation(int which) {
	switch (which) {
	case 1:
		if (is_state(NPCAnimationState::special_1)) { return; }
		Mobile::p_animatable.set_animation("special_1");
		state_function = [this]() { return this->update_special_1(); };
		break;
	case 2:
		if (is_state(NPCAnimationState::special_2)) { return; }
		Mobile::p_animatable.set_animation("special_2");
		state_function = [this]() { return this->update_special_2(); };
		break;
	case 3:
		if (is_state(NPCAnimationState::special_3)) { return; }
		Mobile::p_animatable.set_animation("special_3");
		state_function = [this]() { return this->update_special_3(); };
		break;
	}
}

void NPC::set_busy() {
	state_function = {[this]() { return this->update_busy(); }};
	set_flag(NPCFlags::busy);
	request(NPCAnimationState::busy);
	Mobile::p_animatable.set_animation("busy");
}

void NPC::use_portal(world::Map& map) {
	if (is_hidden()) { return; }
	for (auto portal : map.get_entities<Portal>()) {
		if (std::abs(portal->bounding_box.get_center().x - get_collider().get_center().x) < 4.f) {
			portal->open();
			request(NPCAnimationState::inspect);
			get_collider().physics.zero();
			if (!m_disappear.running()) { m_disappear.start(); }
		}
	}
}

void NPC::walk() { request(NPCAnimationState::walk); }

void NPC::set_position(sf::Vector2f pos) {
	get_collider().physics.position = pos;
	if (m_vehicle) { m_vehicle->set_position(pos); }
}

void NPC::set_position_from_scaled(sf::Vector2f scaled_pos) {
	auto new_pos = scaled_pos * constants::f_cell_size;
	auto round = static_cast<int>(get_collider().dimensions.y) % 32;
	new_pos.y += static_cast<float>(constants::f_cell_size - round);
	set_position(new_pos);
}

fsm::StateFunction NPC::update_idle() {
	p_state.actual = NPCAnimationState::idle;
	if (m_busy_timer.is_almost_complete()) { request(NPCAnimationState::busy); }
	if (change_state(NPCAnimationState::stagger, "stagger")) { return std::move(fsm::StateFunction{NPC_BIND(update_stagger)}); }
	if (change_state(NPCAnimationState::special_1, "special_1")) { return std::move(fsm::StateFunction{NPC_BIND(update_special_1)}); }
	if (change_state(NPCAnimationState::special_2, "special_2")) { return std::move(fsm::StateFunction{NPC_BIND(update_special_2)}); }
	if (change_state(NPCAnimationState::special_3, "special_3")) { return std::move(fsm::StateFunction{NPC_BIND(update_special_3)}); }
	if (change_state(NPCAnimationState::busy, "busy")) { return std::move(fsm::StateFunction{NPC_BIND(update_busy)}); }
	if (change_state(NPCAnimationState::inspect, "inspect")) { return std::move(fsm::StateFunction{NPC_BIND(update_inspect)}); }
	if (change_state(NPCAnimationState::fall, "fall")) { return std::move(fsm::StateFunction{NPC_BIND(update_fall)}); }
	if (change_state(NPCAnimationState::turn, "turn")) { return std::move(fsm::StateFunction{NPC_BIND(update_turn)}); }
	if (collider.has_value()) {
		if (get_collider().grounded()) {
			if (change_state(NPCAnimationState::walk, "walk")) { return std::move(fsm::StateFunction{NPC_BIND(update_walk)}); }
		}
	}
	return std::move(fsm::StateFunction{NPC_BIND(update_idle)});
}

fsm::StateFunction NPC::update_walk() {
	p_state.actual = NPCAnimationState::walk;
	if (collider.has_value()) { get_collider().physics.acceleration.x = m_walk_speed * directions.actual.as_float(); }
	if (change_state(NPCAnimationState::stagger, "stagger")) { return std::move(fsm::StateFunction{NPC_BIND(update_stagger)}); }
	if (change_state(NPCAnimationState::busy, "busy")) { return std::move(fsm::StateFunction{NPC_BIND(update_busy)}); }
	if (change_state(NPCAnimationState::inspect, "inspect")) { return std::move(fsm::StateFunction{NPC_BIND(update_inspect)}); }
	if (!m_state.test(NPCState::random_walk)) {
		if (change_state(NPCAnimationState::turn, "turn")) { return std::move(fsm::StateFunction{NPC_BIND(update_turn)}); }
	} else {
		if (Mobile::p_animatable.animation.is_complete()) {
			request(NPCAnimationState::idle);
			m_state.reset(NPCState::random_walk);
			if (random::coin_flip()) {
				directions.desired.flip();
				request(NPCAnimationState::turn);
			}
			if (change_state(NPCAnimationState::turn, "turn")) { return std::move(fsm::StateFunction{NPC_BIND(update_turn)}); }
			if (has_flag_set(NPCFlags::busy)) {
				request(NPCAnimationState::busy);
				if (change_state(NPCAnimationState::busy, "busy")) { return std::move(fsm::StateFunction{NPC_BIND(update_busy)}); }
			}
		}
	}
	if (change_state(NPCAnimationState::idle, "idle")) { return std::move(fsm::StateFunction{NPC_BIND(update_idle)}); }
	return std::move(fsm::StateFunction{NPC_BIND(update_walk)});
}

fsm::StateFunction NPC::update_inspect() {
	p_state.actual = NPCAnimationState::inspect;
	if (change_state(NPCAnimationState::stagger, "stagger")) { return std::move(fsm::StateFunction{NPC_BIND(update_stagger)}); }
	if (change_state(NPCAnimationState::idle, "idle")) { return std::move(fsm::StateFunction{NPC_BIND(update_idle)}); }
	if (change_state(NPCAnimationState::turn, "turn")) { return std::move(fsm::StateFunction{NPC_BIND(update_turn)}); }
	if (change_state(NPCAnimationState::walk, "walk")) { return std::move(fsm::StateFunction{NPC_BIND(update_walk)}); }
	if (change_state(NPCAnimationState::special_1, "special_1")) { return std::move(fsm::StateFunction{NPC_BIND(update_special_1)}); }
	return std::move(fsm::StateFunction{NPC_BIND(update_inspect)});
}

fsm::StateFunction NPC::update_turn() {
	p_state.actual = NPCAnimationState::turn;
	directions.desired.lock();
	if (Mobile::p_animatable.animation.is_complete()) {
		request_flip();
		if (change_state(NPCAnimationState::stagger, "stagger")) { return std::move(fsm::StateFunction{NPC_BIND(update_stagger)}); }
		if (change_state(NPCAnimationState::walk, "walk")) { return std::move(fsm::StateFunction{NPC_BIND(update_walk)}); }
		request(NPCAnimationState::idle);
		if (change_state(NPCAnimationState::idle, "idle")) { return std::move(fsm::StateFunction{NPC_BIND(update_idle)}); }
	}
	return std::move(fsm::StateFunction{NPC_BIND(update_turn)});
}

fsm::StateFunction NPC::update_fall() {
	p_state.actual = NPCAnimationState::fall;
	if (collider.has_value()) {
		if (get_collider().grounded()) {
			request(NPCAnimationState::land);
			if (change_state(NPCAnimationState::land, "land")) { return std::move(fsm::StateFunction{NPC_BIND(update_land)}); }
		}
	}
	return std::move(fsm::StateFunction{NPC_BIND(update_fall)});
}

fsm::StateFunction NPC::update_land() {
	p_state.actual = NPCAnimationState::land;
	if (Mobile::p_animatable.animation.is_complete()) {
		if (change_state(NPCAnimationState::turn, "turn")) { return std::move(fsm::StateFunction{NPC_BIND(update_turn)}); }
		if (change_state(NPCAnimationState::walk, "walk")) { return std::move(fsm::StateFunction{NPC_BIND(update_walk)}); }
		request(NPCAnimationState::idle);
		if (change_state(NPCAnimationState::idle, "idle")) { return std::move(fsm::StateFunction{NPC_BIND(update_idle)}); }
	}
	return std::move(fsm::StateFunction{NPC_BIND(update_land)});
}

fsm::StateFunction NPC::update_busy() {
	p_state.actual = NPCAnimationState::busy;
	if (change_state(NPCAnimationState::stagger, "stagger")) { return std::move(fsm::StateFunction{NPC_BIND(update_stagger)}); }
	if (change_state(NPCAnimationState::respond, "respond")) { return std::move(fsm::StateFunction{NPC_BIND(update_respond)}); }
	if (change_state(NPCAnimationState::idle, "idle")) { return std::move(fsm::StateFunction{NPC_BIND(update_idle)}); }
	if (change_state(NPCAnimationState::walk, "walk")) { return std::move(fsm::StateFunction{NPC_BIND(update_walk)}); }
	if (has_flag_set(NPCFlags::cutscene)) {
		if (change_state(NPCAnimationState::turn, "turn")) { return std::move(fsm::StateFunction{NPC_BIND(update_turn)}); }
	}
	return std::move(fsm::StateFunction{NPC_BIND(update_busy)});
}

fsm::StateFunction NPC::update_stagger() {
	p_state.actual = NPCAnimationState::stagger;
	if (Mobile::p_animatable.animation.is_complete()) {
		if (change_state(NPCAnimationState::turn, "turn")) { return std::move(fsm::StateFunction{NPC_BIND(update_turn)}); }
		if (change_state(NPCAnimationState::walk, "walk")) { return std::move(fsm::StateFunction{NPC_BIND(update_walk)}); }
		request(NPCAnimationState::idle);
		if (change_state(NPCAnimationState::idle, "idle")) { return std::move(fsm::StateFunction{NPC_BIND(update_idle)}); }
	}
	return std::move(fsm::StateFunction{NPC_BIND(update_stagger)});
}

fsm::StateFunction NPC::update_respond() {
	p_state.actual = NPCAnimationState::respond;
	if (Mobile::p_animatable.animation.just_started()) { m_busy_timer.start(); }
	if (Mobile::p_animatable.animation.is_complete()) {
		if (change_state(NPCAnimationState::turn, "turn")) { return std::move(fsm::StateFunction{NPC_BIND(update_turn)}); }
		request(NPCAnimationState::idle);
		if (change_state(NPCAnimationState::idle, "idle")) { return std::move(fsm::StateFunction{NPC_BIND(update_idle)}); }
	}
	return std::move(fsm::StateFunction{NPC_BIND(update_respond)});
}

fsm::StateFunction NPC::update_special_1() {
	p_state.actual = NPCAnimationState::special_1;
	if (change_state(NPCAnimationState::stagger, "stagger")) { return std::move(fsm::StateFunction{NPC_BIND(update_stagger)}); }
	if (change_state(NPCAnimationState::turn, "turn")) { return std::move(fsm::StateFunction{NPC_BIND(update_turn)}); }
	if (change_state(NPCAnimationState::idle, "idle")) { return std::move(fsm::StateFunction{NPC_BIND(update_idle)}); }
	if (change_state(NPCAnimationState::walk, "walk")) { return std::move(fsm::StateFunction{NPC_BIND(update_walk)}); }
	return std::move(fsm::StateFunction{NPC_BIND(update_special_1)});
}

fsm::StateFunction NPC::update_special_2() {
	p_state.actual = NPCAnimationState::special_2;
	if (change_state(NPCAnimationState::stagger, "stagger")) { return std::move(fsm::StateFunction{NPC_BIND(update_stagger)}); }
	return std::move(fsm::StateFunction{NPC_BIND(update_special_2)});
}

fsm::StateFunction NPC::update_special_3() {
	p_state.actual = NPCAnimationState::special_3;
	if (change_state(NPCAnimationState::stagger, "stagger")) { return std::move(fsm::StateFunction{NPC_BIND(update_stagger)}); }
	if (Mobile::p_animatable.animation.is_complete()) {
		if (change_state(NPCAnimationState::turn, "turn")) { return std::move(fsm::StateFunction{NPC_BIND(update_turn)}); }
		if (change_state(NPCAnimationState::idle, "idle")) { return std::move(fsm::StateFunction{NPC_BIND(update_idle)}); }
		if (change_state(NPCAnimationState::walk, "walk")) { return std::move(fsm::StateFunction{NPC_BIND(update_walk)}); }
	}
	return std::move(fsm::StateFunction{NPC_BIND(update_special_3)});
}

bool NPC::change_state(NPCAnimationState next, std::string_view to) {
	if (p_state.desired == next) {
		Mobile::p_animatable.set_animation(to);
		return true;
	}
	return false;
}

NPCSchedule::NPCSchedule(dj::Json const& in) {
	destinations.insert({TimeOfDay::night, in["night"].as<int>()});
	destinations.insert({TimeOfDay::dawn, in["dawn"].as<int>()});
	destinations.insert({TimeOfDay::day, in["day"].as<int>()});
	destinations.insert({TimeOfDay::dusk, in["dusk"].as<int>()});
}

} // namespace fornani
