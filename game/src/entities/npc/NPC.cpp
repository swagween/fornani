
#include "fornani/entities/npc/NPC.hpp"
#include "fornani/entities/player/Player.hpp"
#include "fornani/gui/Console.hpp"
#include "fornani/service/ServiceProvider.hpp"

namespace fornani::npc {

static int s_voice_cue{};
static bool b_cue{};
static void voice_cue(int index) {
	b_cue = true;
	s_voice_cue = index;
}

NPC::NPC(automa::ServiceProvider& svc, std::string_view label)
	: Mobile(svc, "npc_" + std::string{label}, {svc.data.npc[label]["sprite_dimensions"][0].as<int>(), svc.data.npc[label]["sprite_dimensions"][1].as<int>()}), m_label(label),
	  m_indicator(svc.assets.get_texture("arrow_indicator"), {16, 16}), id{svc.data.npc[label]["id"].as<int>()} {

	svc.events.register_event(std::make_unique<Event<int>>("VoiceCue", &voice_cue));

	m_indicator.set_origin({0.f, 48.f});
	m_indicator.push_params("neutral", {0, 15, 16, 0, true});
	m_indicator.end();
	auto const& in_data = svc.data.npc[label];

	m_offset = sf::Vector2f{svc.data.npc[label]["sprite_offset"][0].as<float>(), svc.data.npc[label]["sprite_offset"][1].as<float>()};
	if (in_data["vendor"] && svc.data.marketplace.contains(id)) { vendor = &svc.data.marketplace.at(id); }

	collider = shape::Collider({svc.data.npc[label]["dimensions"][0].as<float>(), svc.data.npc[label]["dimensions"][1].as<float>()});
	collider.sync_components();

	collider.physics.set_global_friction(0.99f);
	collider.stats.GRAV = 6.2f;

	for (auto const& in_anim : in_data["animation"].as_array()) {
		m_params.push_back({in_anim["parameters"][0].as<int>(), in_anim["parameters"][1].as<int>(), in_anim["parameters"][2].as<int>(), in_anim["parameters"][3].as<int>(), in_anim["parameters"][4].as_bool()});
	}
	if (!m_params.empty()) { set_parameters(m_params.at(0)); }
}

void NPC::update(automa::ServiceProvider& svc, world::Map& map, std::optional<std::unique_ptr<gui::Console>>& console, player::Player& player) {
	tick();
	face_player(player);
	if (piggybacking()) { current_location = -1; }
	svc.data.set_npc_location(id, current_location);
	if (state_flags.test(NPCState::hidden)) { return; }
	svc.player_dat.piggy_id == id ? state_flags.set(NPCState::piggybacking) : state_flags.reset(NPCState::piggybacking);
	if (abs(collider.physics.velocity.x) > physical.walk_threshold) {}

	m_indicator.update(collider.physics.position);

	collider.update(svc);
	collider.detect_map_collision(map);
	collider.reset();
	collider.physics.acceleration = {};

	console ? state_flags.set(NPCState::talking) : state_flags.reset(NPCState::talking);
	if (player.collider.bounding_box.overlaps(collider.bounding_box) || (triggers.test(NPCTrigger::distant_interact) && state_flags.test(NPCState::force_interact))) {
		if (!state_flags.test(NPCState::engaged)) { triggers.set(NPCTrigger::engaged); }
		state_flags.set(NPCState::engaged);
		if ((player.controller.inspecting() || state_flags.test(NPCState::force_interact)) && !conversations.empty()) { start_conversation(svc, console); }
	} else {
		state_flags.reset(NPCState::engaged);
	}
	if (state_flags.test(NPCState::engaged) && triggers.consume(NPCTrigger::engaged) && m_indicator.complete()) { m_indicator.set_params("neutral", true); }

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
}

void NPC::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f campos) {
	if (state_flags.test(NPCState::hidden)) { return; }
	Animatable::set_position(collider.physics.position + m_offset - campos);
	if (svc.greyblock_mode()) {
		collider.render(win, campos);
	} else {
		win.draw(*this);
	}
	m_indicator.render(svc, win, campos);
}

void NPC::force_engage() {
	state_flags.set(NPCState::engaged);
	state_flags.set(NPCState::cutscene);
	triggers.set(NPCTrigger::distant_interact);
	state_flags.set(NPCState::force_interact);
}

void NPC::set_position(sf::Vector2f pos) { collider.physics.position = pos; }

void NPC::set_position_from_scaled(sf::Vector2f scaled_pos) {
	auto new_pos = scaled_pos * constants::f_cell_size;
	auto round = static_cast<int>(collider.dimensions.y) % 32;
	new_pos.y += static_cast<float>(constants::f_cell_size - round);
	set_position(new_pos);
}

void NPC::set_id(int new_id) { id = new_id; }

void NPC::start_conversation(automa::ServiceProvider& svc, std::optional<std::unique_ptr<gui::Console>>& console) {
	for (auto& conv : conversations) { NANI_LOG_DEBUG(m_logger, "Convos: {}", conv); }
	state_flags.set(NPCState::introduced);
	std::string name = std::string(m_label);
	std::string target = std::to_string(conversations.front());
	NANI_LOG_DEBUG(m_logger, "Starting convo: {}", target);
	console = std::make_unique<gui::Console>(svc, svc.text.npc[name], target, gui::OutputType::gradual);
	console.value()->include_portrait(id);
}

void NPC::push_conversation(int convo) {
	conversations.push_back(convo);
	NANI_LOG_DEBUG(m_logger, "Pushed convo: {}", convo);
}

void NPC::pop_conversation() {
	if (conversations.size() > 1) { conversations.pop_front(); }
	NANI_LOG_DEBUG(m_logger, "popped");
	state_flags.reset(NPCState::cutscene); // this function should only be called for cutscenes
}

void NPC::flush_conversations() {
	NANI_LOG_DEBUG(m_logger, "flushed");
	conversations.clear();
}

} // namespace fornani::npc
