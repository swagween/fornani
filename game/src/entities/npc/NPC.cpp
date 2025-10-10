
#include "fornani/entities/npc/NPC.hpp"
#include <fornani/gui/console/Console.hpp>
#include "fornani/entities/player/Player.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/world/Map.hpp"

namespace fornani::npc {

static int s_voice_cue{};
static bool b_cue{};
static bool b_pop{};
static int b_index{};
static void voice_cue(int index) {
	b_cue = true;
	s_voice_cue = index;
}
static void pop_convo(int index) {
	b_index = index;
	b_pop = true;
}

NPC::NPC(automa::ServiceProvider& svc, std::string_view label)
	: Mobile(svc, "npc_" + std::string{label}, {svc.data.npc[label]["sprite_dimensions"][0].as<int>(), svc.data.npc[label]["sprite_dimensions"][1].as<int>()}), m_label(label), m_indicator(svc, "arrow_indicator", {16, 16}),
	  m_id{svc.data.npc[label]["id"].as<int>()}, m_current_conversation{1} {

	center();
	m_indicator.center();

	svc.events.register_event(std::make_unique<Event<int>>("VoiceCue", &voice_cue));
	svc.events.register_event(std::make_unique<Event<int>>("PopConversation", &pop_convo));

	auto const& in_data = svc.data.npc[label];

	m_offset = sf::Vector2f{svc.data.npc[label]["sprite_offset"][0].as<float>(), svc.data.npc[label]["sprite_offset"][1].as<float>()};
	if (in_data["vendor"] && svc.data.marketplace.contains(get_id())) { vendor = &svc.data.marketplace.at(get_id()); }

	collider = shape::Collider({svc.data.npc[label]["dimensions"][0].as<float>(), svc.data.npc[label]["dimensions"][1].as<float>()});
	collider.sync_components();

	collider.physics.set_friction_componentwise({0.95f, 0.995f});
	collider.stats.GRAV = 16.2f;

	for (auto const& in_anim : in_data["animation"].as_array()) {
		m_params.insert({in_anim["label"].as_string(), {in_anim["parameters"][0].as<int>(), in_anim["parameters"][1].as<int>(), in_anim["parameters"][2].as<int>(), in_anim["parameters"][3].as<int>(), in_anim["parameters"][4].as_bool()}});
		if (in_anim["label"].as_string() == "turn") { p_flags.set(NPCFlags::has_turn_animation); }
	}
	if (m_params.contains("idle")) { set_parameters(m_params.at("idle")); }
	request(NPCAnimationState::idle);
	directions.actual.lnr = LNR::left;
}

void NPC::update(automa::ServiceProvider& svc, world::Map& map, std::optional<std::unique_ptr<gui::Console>>& console, player::Player& player) {
	face_player(player);
	if (piggybacking()) { current_location = -1; }
	svc.data.set_npc_location(get_id(), current_location);
	if (hidden()) { return; }
	// NANI_LOG_DEBUG(m_logger, "NPC exists!");
	// svc.player_dat.piggy_id == get_id() ? state_flags.set(NPCState::piggybacking) : state_flags.reset(NPCState::piggybacking);
	if (abs(collider.physics.velocity.x) > physical.walk_threshold) {}

	m_indicator.tick();

	if (b_pop && b_index == get_id()) {
		m_current_conversation.modulate(1);
		// NANI_LOG_DEBUG(m_logger, "Current conversation N: {}", m_current_conversation.get());
		// NANI_LOG_DEBUG(m_logger, "Current order N: {}", m_current_conversation.get_order());
		b_pop = false;
	}

	collider.update(svc);
	collider.detect_map_collision(map);
	for (auto& destructible : map.destructibles) {
		if (!destructible.ignore_updates()) { collider.handle_collider_collision(destructible.get_bounding_box()); }
	}
	collider.reset();
	collider.physics.acceleration = {};

	console ? state_flags.set(NPCState::talking) : state_flags.reset(NPCState::talking);
	if (player.collider.bounding_box.overlaps(collider.bounding_box) || (triggers.test(NPCTrigger::distant_interact) && state_flags.test(NPCState::force_interact))) {
		if (!state_flags.test(NPCState::engaged)) { triggers.set(NPCTrigger::engaged); }
		state_flags.set(NPCState::engaged);
		if ((player.controller.inspecting() || state_flags.test(NPCState::force_interact)) && !conversations.empty()) {
			start_conversation(svc, console);
			player.set_busy(true);
		}
	} else {
		state_flags.reset(NPCState::engaged);
	}
	if (state_flags.test(NPCState::engaged) && triggers.consume(NPCTrigger::engaged)) { m_indicator.set_parameters(anim::Parameters{0, 15, 16, 0, true}); }

	if (b_cue) {
		svc.soundboard.flags.npc.set(static_cast<audio::NPC>(s_voice_cue));
		b_cue = false;
	}

	if (state_flags.test(NPCState::talking)) {
		svc.camera_controller.free();
	} else {
		svc.camera_controller.constrain();
	}
	triggers = {};

	if (!collider.grounded()) { request(NPCAnimationState::fall); }
	if (directions.actual.lnr != directions.desired.lnr) { request(NPCAnimationState::turn); }

	state_function = state_function();
}

void NPC::post_update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {
	if (hidden()) { return; }
	Mobile::post_update(svc, map, player);
}

void NPC::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f campos) {
	if (hidden()) { return; }
	Animatable::set_position(collider.get_center() + m_offset - campos);
	auto indicator_offset = sf::Vector2f{0.f, -constants::f_cell_size};
	m_indicator.set_position(collider.get_top() + indicator_offset - campos);
	if (svc.greyblock_mode()) {
		collider.render(win, campos);
	} else {
		win.draw(*this);
	}
	win.draw(m_indicator);
}

void NPC::force_engage() {
	state_flags.set(NPCState::engaged);
	state_flags.set(NPCState::cutscene);
	triggers.set(NPCTrigger::distant_interact);
	state_flags.set(NPCState::force_interact);
}

void NPC::disengage() {
	state_flags.reset(NPCState::engaged);
	state_flags.reset(NPCState::cutscene);
	triggers.reset(NPCTrigger::distant_interact);
	state_flags.reset(NPCState::force_interact);
}

void NPC::set_position(sf::Vector2f pos) { collider.physics.position = pos; }

void NPC::set_position_from_scaled(sf::Vector2f scaled_pos) {
	auto new_pos = scaled_pos * constants::f_cell_size;
	auto round = static_cast<int>(collider.dimensions.y) % 32;
	new_pos.y += static_cast<float>(constants::f_cell_size - round);
	set_position(new_pos);
}

void NPC::start_conversation(automa::ServiceProvider& svc, std::optional<std::unique_ptr<gui::Console>>& console) {
	state_flags.set(NPCState::introduced);
	std::string name = std::string(m_label);
	std::string target = std::to_string(conversations.at(m_current_conversation.get()));
	if (!svc.text.npc[name][target].is_object()) {
		NANI_LOG_ERROR(m_logger, "Tried to start a conversation that doesn't exist!");
		return;
	}
	console = std::make_unique<gui::Console>(svc, svc.text.npc[name][target], static_cast<gui::OutputType>(svc.text.npc[name][target]["output"].as<int>()));
	console.value()->include_portrait(get_id());
}

void NPC::push_conversation(int convo) {
	conversations.push_back(convo);
	m_current_conversation.set_order(static_cast<int>(conversations.size()));
	// NANI_LOG_DEBUG(m_logger, "Current order Push: {}", static_cast<int>(conversations.size()));
}

void NPC::pop_conversation() {
	if (conversations.size() > 1) { conversations.pop_front(); }
	state_flags.reset(NPCState::cutscene); // this function should only be called for cutscenes
}

void NPC::flush_conversations() { conversations.clear(); }

void NPC::walk() { request(NPCAnimationState::walk); }

fsm::StateFunction NPC::update_idle() {
	p_state.actual = NPCAnimationState::idle;
	if (change_state(NPCAnimationState::fall, get_params("fall"))) { return NPC_BIND(update_fall); }
	if (change_state(NPCAnimationState::turn, get_params("turn"))) { return NPC_BIND(update_turn); }
	if (collider.grounded()) {
		if (change_state(NPCAnimationState::walk, get_params("walk"))) { return NPC_BIND(update_walk); }
	}
	return NPC_BIND(update_idle);
}

fsm::StateFunction NPC::update_walk() {
	p_state.actual = NPCAnimationState::walk;
	collider.physics.acceleration.x = 2.8f;
	if (change_state(NPCAnimationState::turn, get_params("turn"))) { return NPC_BIND(update_turn); }
	if (change_state(NPCAnimationState::idle, get_params("idle"))) { return NPC_BIND(update_idle); }
	return NPC_BIND(update_walk);
}

fsm::StateFunction NPC::update_inspect() {
	p_state.actual = NPCAnimationState::inspect;
	if (change_state(NPCAnimationState::turn, get_params("turn"))) { return NPC_BIND(update_turn); }
	if (change_state(NPCAnimationState::walk, get_params("walk"))) { return NPC_BIND(update_walk); }
	return NPC_BIND(update_inspect);
}

fsm::StateFunction NPC::update_turn() {
	p_state.actual = NPCAnimationState::turn;
	directions.desired.lock();
	if (animation.is_complete()) {
		request_flip();
		if (change_state(NPCAnimationState::walk, get_params("walk"))) { return NPC_BIND(update_walk); }
		request(NPCAnimationState::idle);
		if (change_state(NPCAnimationState::idle, get_params("idle"))) { return NPC_BIND(update_idle); }
	}
	return NPC_BIND(update_turn);
}

fsm::StateFunction NPC::update_fall() {
	p_state.actual = NPCAnimationState::fall;
	if (collider.grounded()) {
		request(NPCAnimationState::land);
		if (change_state(NPCAnimationState::land, get_params("land"))) { return NPC_BIND(update_land); }
	}
	return NPC_BIND(update_fall);
}

fsm::StateFunction NPC::update_land() {
	p_state.actual = NPCAnimationState::land;
	if (animation.is_complete()) {
		if (change_state(NPCAnimationState::turn, get_params("turn"))) { return NPC_BIND(update_turn); }
		if (change_state(NPCAnimationState::idle, get_params("idle"))) { return NPC_BIND(update_idle); }
		if (change_state(NPCAnimationState::walk, get_params("walk"))) { return NPC_BIND(update_walk); }
	}
	return NPC_BIND(update_land);
}

bool NPC::change_state(NPCAnimationState next, anim::Parameters params) {
	if (p_state.desired == next) {
		set_parameters(params);
		return true;
	}
	return false;
}

} // namespace fornani::npc
