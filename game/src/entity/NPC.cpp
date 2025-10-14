
#include <fornani/entities/player/Player.hpp>
#include <fornani/entity/NPC.hpp>
#include <fornani/gui/console/Console.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/world/Map.hpp>

namespace fornani {

static int s_voice_cue{};
static bool b_cue{};
static bool b_pop{};
static bool b_piggy{};
static int b_index{};
static int b_piggy_id{};
static void voice_cue(int index) {
	b_cue = true;
	s_voice_cue = index;
}
static void pop_convo(int index) {
	b_index = index;
	b_pop = true;
}
static void piggyback_me(int index) {
	b_piggy_id = index;
	b_piggy = true;
}

NPC::NPC(automa::ServiceProvider& svc, dj::Json const& in)
	: Entity(svc, in, "npcs"), Mobile(svc, "npc_" + std::string{in["label"].as_string()}, {svc.data.npc[in["label"].as_string()]["sprite_dimensions"][0].as<int>(), svc.data.npc[in["label"].as_string()]["sprite_dimensions"][1].as<int>()}),
	  m_label(in["label"].as_string()), m_indicator(svc, "arrow_indicator", {16, 16}), m_id{svc.data.npc[in["label"].as_string()]["id"].as<int>()}, m_current_conversation{1}, m_services{&svc} {
	unserialize(in);
	repeatable = false;
	copyable = false;
	Mobile::center();
	Entity::center();
	m_indicator.center();

	NANI_LOG_DEBUG(Entity::m_logger, "Created NPC with label {}", m_label);

	svc.events.register_event(std::make_unique<Event<int>>("VoiceCue", &voice_cue));
	svc.events.register_event(std::make_unique<Event<int>>("PopConversation", &pop_convo));
	svc.events.register_event(std::make_unique<Event<int>>("PiggybackNPC", &piggyback_me));

	auto const& in_data = svc.data.npc[m_label];

	m_offset = sf::Vector2f{svc.data.npc[m_label]["sprite_offset"][0].as<float>(), svc.data.npc[m_label]["sprite_offset"][1].as<float>()};
	if (in_data["vendor"] && svc.data.marketplace.contains(get_id())) { vendor = &svc.data.marketplace.at(get_id()); }

	collider = shape::Collider({svc.data.npc[m_label]["dimensions"][0].as<float>(), svc.data.npc[m_label]["dimensions"][1].as<float>()});
	collider.sync_components();

	collider.physics.set_friction_componentwise({0.95f, 0.995f});
	collider.stats.GRAV = 16.2f;

	for (auto const& in_anim : in_data["animation"].as_array()) {
		m_params.insert({in_anim["label"].as_string(), {in_anim["parameters"][0].as<int>(), in_anim["parameters"][1].as<int>(), in_anim["parameters"][2].as<int>(), in_anim["parameters"][3].as<int>(), in_anim["parameters"][4].as_bool()}});
		if (in_anim["label"].as_string() == "turn") { m_flags.set(NPCFlags::has_turn_animation); }
	}
	if (m_params.contains("idle")) { Mobile::set_parameters(m_params.at("idle")); }
	request(NPCAnimationState::idle);
	directions.actual.lnr = LNR::left;

	set_position_from_scaled(sf::Vector2f{in["position"][0].as<float>(), in["position"][1].as<float>()});
	auto push = true;
	auto fail_tag = std::string{};
	if (in["contingencies"].is_array()) {
		for (auto const& contingency : in["contingencies"].as_array()) {
			auto cont = QuestContingency{contingency};
			if (!svc.quest_table.are_contingencies_met({cont})) {
				hide();
				fail_tag = contingency["tag"].as_string();
				NANI_LOG_DEBUG(Entity::m_logger, "NPC did not meet contingency for quest {}.", fail_tag);
			}
		}
	}
	auto npc_state = svc.quest_table.get_quest_progression("npc_dialogue", {m_label, in["id"].as<int>()});
	NANI_LOG_DEBUG(Entity::m_logger, "NPC State: {}", npc_state);
	for (auto const& convo : in["suites"][npc_state].as_array()) {
		push_conversation(convo.as<int>());
		NANI_LOG_DEBUG(Entity::m_logger, "Pushed conversation {}", convo.as<int>());
	}

	if (m_hidden) { m_state.set(NPCState::hidden); }
	if (m_background) { m_flags.set(NPCFlags::background); }
}

NPC::NPC(automa::ServiceProvider& svc, int id, std::string_view label, std::vector<std::vector<int>> const suites)
	: Entity(svc, "npcs", id, {1, 1}), Mobile(svc, "npc_" + std::string{label}, {svc.data.npc[label]["sprite_dimensions"][0].as<int>(), svc.data.npc[label]["sprite_dimensions"][1].as<int>()}), m_label(label),
	  m_indicator(svc, "arrow_indicator", {16, 16}), m_id{svc.data.npc[label]["id"].as<int>()}, m_current_conversation{1}, m_suites{suites}, m_services{&svc} {
	repeatable = false;
	copyable = false;
	m_flags.set(NPCFlags::face_player); // default to face player
}

void NPC::serialize(dj::Json& out) {
	Entity::serialize(out);
	out["background"] = m_background;
	out["label"] = m_label;
	out["hidden"] = m_hidden;
	out["face_player"] = m_flags.test(NPCFlags::face_player);
	for (auto& suite : m_suites) {
		auto entry = dj::Json::empty_array();
		for (auto& set : suite) { entry.push_back(set); }
		out["suites"].push_back(entry);
	}
	for (auto& contingency : m_contingencies) { contingency.serialize(out["contingencies"]); }
}

void NPC::unserialize(dj::Json const& in) {
	Entity::unserialize(in);
	m_background = in["background"].as_bool();
	if (in["face_player"].is_object()) {
		in["face_player"].as_bool() ? m_flags.set(NPCFlags::face_player) : m_flags.reset(NPCFlags::face_player);
	} else {
		m_flags.set(NPCFlags::face_player); // defautlt to facing player
	}
	m_label = in["label"].as_string();
	m_hidden = in["hidden"].as_bool();
	for (auto const& suite : in["suites"].as_array()) {
		auto entry = std::vector<int>{};
		for (auto const& set : suite.as_array()) { entry.push_back(set.as<int>()); }
		m_suites.push_back(entry);
	}
	for (auto const& contingency : in["contingencies"].as_array()) { m_contingencies.push_back(QuestContingency(contingency)); }
}

void NPC::expose() {
	Entity::expose();
	ImGui::Checkbox("Hidden?", &m_hidden);
	for (auto [i, ct] : std::views::enumerate(m_contingencies)) {
		ImGui::PushID(i);
		ImGui::Text("Contingency %i: [%s, %i]", i, ct.tag.c_str(), ct.requirement);
		ImGui::PopID();
	}
}

void NPC::update([[maybe_unused]] automa::ServiceProvider& svc, [[maybe_unused]] world::Map& map, [[maybe_unused]] std::optional<std::unique_ptr<gui::Console>>& console, [[maybe_unused]] player::Player& player) {
	if (m_flags.test(NPCFlags::face_player)) { face_player(player); }
	svc.data.set_npc_location(m_id.get(), current_location);

	if (b_piggy && b_piggy_id == m_id.get()) {
		NANI_LOG_DEBUG(Entity::m_logger, "Started piggybacking NPC {}", b_piggy_id);
		NANI_LOG_DEBUG(Entity::m_logger, "ID from text was {}", b_piggy_id);
		NANI_LOG_DEBUG(Entity::m_logger, "ID from NPC was {}", m_id.get());
		is_hidden() ? unhide() : hide();
		player.piggyback(m_id.get());
		b_piggy = false;
	}

	if (is_hidden()) { return; }

	m_indicator.tick();

	if (b_pop && b_index == m_id.get()) {
		m_current_conversation.modulate(1);
		NANI_LOG_DEBUG(Entity::m_logger, "Current conversation N: {}", m_current_conversation.get());
		NANI_LOG_DEBUG(Entity::m_logger, "Current order N: {}", m_current_conversation.get_order());
		b_pop = false;
	}
	if (b_pop) {
		NANI_LOG_DEBUG(Entity::m_logger, "Conversation Pop Mismatch!");
		NANI_LOG_DEBUG(Entity::m_logger, "ID from text was {}", b_index);
		NANI_LOG_DEBUG(Entity::m_logger, "ID from NPC was {}", m_id.get());
		b_pop = false;
	}

	collider.update(svc);
	collider.detect_map_collision(map);
	for (auto& destructible : map.destructibles) {
		if (!destructible.ignore_updates()) { collider.handle_collider_collision(destructible.get_bounding_box()); }
	}
	collider.reset();
	collider.physics.acceleration = {};

	console ? m_state.set(NPCState::talking) : m_state.reset(NPCState::talking);
	if (player.collider.bounding_box.overlaps(collider.bounding_box) || (m_state.test(NPCState::distant_interact) && m_state.test(NPCState::force_interact))) {
		if (!m_state.test(NPCState::engaged)) { m_state.set(NPCState::just_engaged); }
		m_state.set(NPCState::engaged);
		if ((player.controller.inspecting() || m_state.test(NPCState::force_interact)) && !conversations.empty()) {
			start_conversation(svc, console);
			player.set_busy(true);
		}
	} else {
		m_state.reset(NPCState::engaged);
	}
	if (m_state.test(NPCState::engaged) && m_state.consume(NPCState::just_engaged)) { m_indicator.set_parameters(anim::Parameters{0, 15, 16, 0, true}); }

	if (b_cue) {
		svc.soundboard.flags.npc.set(static_cast<audio::NPC>(s_voice_cue));
		b_cue = false;
	}

	if (m_state.test(NPCState::talking)) {
		svc.camera_controller.free();
	} else {
		svc.camera_controller.constrain();
	}

	if (!collider.grounded()) { request(NPCAnimationState::fall); }
	if (directions.actual.lnr != directions.desired.lnr) { request(NPCAnimationState::turn); }

	state_function = std::move(state_function());
	if (is_hidden()) { return; }
	Mobile::post_update(svc, map, player);
}

void NPC::render(sf::RenderWindow& win, sf::Vector2f cam, float size) {
	if (m_editor) {
		highlighted ? drawbox.setFillColor(sf::Color{250, 80, 250, 60}) : drawbox.setFillColor(sf::Color::Transparent);
		Entity::render(win, cam, size);
	} else {
		if (is_hidden()) { return; }
		Mobile::set_position(collider.get_center() + m_offset - cam);
		auto indicator_offset = sf::Vector2f{0.f, -constants::f_cell_size};
		m_indicator.set_position(collider.get_top() + indicator_offset - cam);
		if (m_services->greyblock_mode()) {
			collider.render(win, cam);
		} else {
			win.draw(static_cast<Mobile&>(*this));
		}
		win.draw(m_indicator);
	}
}

void NPC::start_conversation(automa::ServiceProvider& svc, std::optional<std::unique_ptr<gui::Console>>& console) {
	m_state.set(NPCState::introduced);
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

void NPC::flush_conversations() { conversations.clear(); }

void NPC::force_engage() {
	m_state.set(NPCState::engaged);
	m_state.set(NPCState::cutscene);
	m_state.set(NPCState::distant_interact);
	m_state.set(NPCState::force_interact);
}

void NPC::disengage() {
	m_state.reset(NPCState::engaged);
	m_state.reset(NPCState::cutscene);
	m_state.reset(NPCState::distant_interact);
	m_state.reset(NPCState::force_interact);
}

void NPC::walk() { request(NPCAnimationState::walk); }

void NPC::set_position(sf::Vector2f pos) { collider.physics.position = pos; }

void NPC::set_position_from_scaled(sf::Vector2f scaled_pos) {
	auto new_pos = scaled_pos * constants::f_cell_size;
	auto round = static_cast<int>(collider.dimensions.y) % 32;
	new_pos.y += static_cast<float>(constants::f_cell_size - round);
	set_position(new_pos);
}

fsm::StateFunction NPC::update_idle() {
	p_state.actual = NPCAnimationState::idle;
	if (change_state(NPCAnimationState::fall, get_params("fall"))) { return std::move(fsm::StateFunction{NPC_BIND(update_fall)}); }
	if (change_state(NPCAnimationState::turn, get_params("turn"))) { return std::move(fsm::StateFunction{NPC_BIND(update_turn)}); }
	if (collider.grounded()) {
		if (change_state(NPCAnimationState::walk, get_params("walk"))) { return std::move(fsm::StateFunction{NPC_BIND(update_walk)}); }
	}
	return std::move(fsm::StateFunction{NPC_BIND(update_idle)});
}

fsm::StateFunction NPC::update_walk() {
	p_state.actual = NPCAnimationState::walk;
	collider.physics.acceleration.x = 2.8f;
	if (change_state(NPCAnimationState::turn, get_params("turn"))) { return std::move(fsm::StateFunction{NPC_BIND(update_turn)}); }
	if (change_state(NPCAnimationState::idle, get_params("idle"))) { return std::move(fsm::StateFunction{NPC_BIND(update_idle)}); }
	return std::move(fsm::StateFunction{NPC_BIND(update_walk)});
}

fsm::StateFunction NPC::update_inspect() {
	p_state.actual = NPCAnimationState::inspect;
	if (change_state(NPCAnimationState::turn, get_params("turn"))) { return std::move(fsm::StateFunction{NPC_BIND(update_turn)}); }
	if (change_state(NPCAnimationState::walk, get_params("walk"))) { return std::move(fsm::StateFunction{NPC_BIND(update_walk)}); }
	return std::move(fsm::StateFunction{NPC_BIND(update_inspect)});
}

fsm::StateFunction NPC::update_turn() {
	p_state.actual = NPCAnimationState::turn;
	directions.desired.lock();
	if (Mobile::animation.is_complete()) {
		request_flip();
		if (change_state(NPCAnimationState::walk, get_params("walk"))) { return std::move(fsm::StateFunction{NPC_BIND(update_walk)}); }
		request(NPCAnimationState::idle);
		if (change_state(NPCAnimationState::idle, get_params("idle"))) { return std::move(fsm::StateFunction{NPC_BIND(update_idle)}); }
	}
	return std::move(fsm::StateFunction{NPC_BIND(update_turn)});
}

fsm::StateFunction NPC::update_fall() {
	p_state.actual = NPCAnimationState::fall;
	if (collider.grounded()) {
		request(NPCAnimationState::land);
		if (change_state(NPCAnimationState::land, get_params("land"))) { return std::move(fsm::StateFunction{NPC_BIND(update_land)}); }
	}
	return std::move(fsm::StateFunction{NPC_BIND(update_fall)});
}

fsm::StateFunction NPC::update_land() {
	p_state.actual = NPCAnimationState::land;
	if (Mobile::animation.is_complete()) {
		if (change_state(NPCAnimationState::turn, get_params("turn"))) { return std::move(fsm::StateFunction{NPC_BIND(update_turn)}); }
		if (change_state(NPCAnimationState::idle, get_params("idle"))) { return std::move(fsm::StateFunction{NPC_BIND(update_idle)}); }
		if (change_state(NPCAnimationState::walk, get_params("walk"))) { return std::move(fsm::StateFunction{NPC_BIND(update_walk)}); }
	}
	return std::move(fsm::StateFunction{NPC_BIND(update_land)});
}

bool NPC::change_state(NPCAnimationState next, anim::Parameters params) {
	if (p_state.desired == next) {
		Mobile::set_parameters(params);
		return true;
	}
	return false;
}

} // namespace fornani
