#include "fornani/entities/npc/NPC.hpp"
#include "fornani/entities/player/Player.hpp"
#include "fornani/gui/Console.hpp"
#include "fornani/service/ServiceProvider.hpp"

namespace fornani::npc {

NPC::NPC(automa::ServiceProvider& svc, std::string_view label, int id)
	: id(id), m_label(label), animation_machine(std::make_unique<NPCAnimation>(svc, id)), m_indicator(svc.assets.get_texture("arrow_indicator"), {32, 32}), m_sprite{svc.assets.get_npc_texture(label.data())} {
	m_indicator.set_origin({0.f, 48.f});
	m_indicator.push_params("neutral", {0, 15, 16, 0, true});
	m_indicator.end();
	auto const& in_data = svc.data.npc[m_label];
	dimensions.x = in_data["dimensions"][0].as<float>();
	dimensions.y = in_data["dimensions"][1].as<float>();
	sprite_dimensions.x = in_data["sprite_dimensions"][0].as<int>();
	sprite_dimensions.y = in_data["sprite_dimensions"][1].as<int>();
	spritesheet_dimensions.x = in_data["spritesheet_dimensions"][0].as<int>();
	spritesheet_dimensions.y = in_data["spritesheet_dimensions"][1].as<int>();
	sprite_offset.x = in_data["sprite_offset"][0].as<float>();
	sprite_offset.y = in_data["sprite_offset"][1].as<float>();

	m_sprite.setOrigin({in_data["sprite_origin"][0].as<float>(), in_data["sprite_origin"][1].as<float>()});

	if (in_data["vendor"] && svc.data.marketplace.contains(id)) { vendor = &svc.data.marketplace.at(id); }

	collider = shape::Collider(dimensions);
	collider.sync_components();

	collider.physics.set_global_friction(0.99f);
	collider.stats.GRAV = 6.2f;

	drawbox.setFillColor(sf::Color::Transparent);
	drawbox.setOutlineThickness(-1);
	drawbox.setSize(dimensions);

	direction.lr = dir::LR::left;
}

void NPC::update(automa::ServiceProvider& svc, world::Map& map, gui::Console& console, player::Player& player) {
	if (piggybacking()) { current_location = -1; }
	svc.data.set_npc_location(id, current_location);
	if (state_flags.test(NPCState::hidden)) { return; }
	svc.player_dat.piggy_id == id ? state_flags.set(NPCState::piggybacking) : state_flags.reset(NPCState::piggybacking);
	direction.lr = (player.collider.physics.position.x < collider.physics.position.x) ? dir::LR::left : dir::LR::right;
	Entity::update(svc, map);
	if (abs(collider.physics.velocity.x) > physical.walk_threshold) { animation_machine->animation_flags.set(NPCAnimState::walk); }
	if (ent_state.test(entity::State::flip)) { animation_machine->animation_flags.set(NPCAnimState::turn); }
	animation_machine->update();

	if (animation_machine->communication_flags.test(NPCCommunication::sprite_flip)) {
		m_sprite.scale({-1.f, 1.f});
		animation_machine->communication_flags.reset(NPCCommunication::sprite_flip);
	}

	m_indicator.update(collider.physics.position);

	collider.update(svc);
	collider.detect_map_collision(map);
	collider.reset();
	collider.physics.acceleration = {};

	console.is_active() ? state_flags.set(NPCState::talking) : state_flags.reset(NPCState::talking);
	if (player.collider.bounding_box.overlaps(collider.bounding_box) || (triggers.test(NPCTrigger::distant_interact) && state_flags.test(NPCState::force_interact))) {
		if (!state_flags.test(NPCState::engaged)) { triggers.set(NPCTrigger::engaged); }
		state_flags.set(NPCState::engaged);
		if ((player.controller.inspecting() || state_flags.test(NPCState::force_interact)) && !conversations.empty()) { start_conversation(svc, console); }
	} else {
		state_flags.reset(NPCState::engaged);
	}
	if (state_flags.test(NPCState::engaged) && triggers.consume(NPCTrigger::engaged) && m_indicator.complete()) { m_indicator.set_params("neutral", true); }

	if (state_flags.test(NPCState::engaged) || state_flags.test(NPCState::cutscene)) {
		// voice cues
		// TODO: properly handle voice cues from the console
		auto voice_cue{1};
		if (voice_cue != 0) {
			// do something clever in Soundboard
		}
	}
	if (!console.is_active() && state_flags.test(NPCState::engaged)) { pop_conversation(); }
	triggers = {};
}

void NPC::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> campos) {
	if (state_flags.test(NPCState::hidden)) { return; }
	m_sprite.setPosition({collider.physics.position.x - campos.x + sprite_offset.x, collider.physics.position.y - campos.y + sprite_offset.y});

	// TODO: switch to AnimatedSprite
	if (spritesheet_dimensions.y > 0) {
		auto u = static_cast<int>(animation_machine->animation.get_frame() / spritesheet_dimensions.y) * sprite_dimensions.x;
		auto v = static_cast<int>(animation_machine->animation.get_frame() % spritesheet_dimensions.y) * sprite_dimensions.y;
		m_sprite.setTextureRect(sf::IntRect({u, v}, {static_cast<int>(sprite_dimensions.x), static_cast<int>(sprite_dimensions.y)}));
	}
	if (svc.greyblock_mode()) {
		collider.render(win, campos);
		drawbox.setPosition(collider.physics.position - campos);
		state_flags.test(NPCState::engaged) ? drawbox.setFillColor(svc.styles.colors.green) : drawbox.setFillColor(svc.styles.colors.dark_orange);
		win.draw(drawbox);
	} else {
		win.draw(m_sprite);
	}
	m_indicator.render(svc, win, campos);
}

void NPC::force_engage() {
	state_flags.set(NPCState::engaged);
	state_flags.set(NPCState::cutscene);
	triggers.set(NPCTrigger::distant_interact);
	state_flags.set(NPCState::force_interact);
}

void NPC::set_position(sf::Vector2<float> pos) { collider.physics.position = pos; }

void NPC::set_position_from_scaled(sf::Vector2<float> scaled_pos) {
	auto new_pos = scaled_pos * 32.f;
	auto round = static_cast<int>(collider.dimensions.y) % 32;
	new_pos.y += static_cast<float>(32.f - round);
	set_position(new_pos);
}

void NPC::set_id(int new_id) { id = new_id; }

void NPC::start_conversation(automa::ServiceProvider& svc, gui::Console& console) {
	state_flags.set(NPCState::introduced);
	console.set_source(svc.text.npc);
	std::string name = std::string(m_label);
	std::string convo = std::string(conversations.front());
	std::string target = name + convo;
	console.load_and_launch(target);
	console.include_portrait(id);
}

void NPC::push_conversation(std::string_view convo) { conversations.push_back(convo); }

void NPC::pop_conversation() {
	if (conversations.size() > 1) { conversations.pop_front(); }
	state_flags.reset(NPCState::cutscene); // this function should only be called for cutscenes
}

void NPC::flush_conversations() { conversations.clear(); }

} // namespace fornani::npc
