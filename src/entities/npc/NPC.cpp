
#include "NPC.hpp"
#include "../../gui/Console.hpp"
#include "../../service/ServiceProvider.hpp"
#include "../player/Player.hpp"

namespace npc {

NPC::NPC(automa::ServiceProvider& svc, int id) : id(id), animation_machine(std::make_unique<NPCAnimation>(svc, id)) {

	label = svc.tables.npc_label.at(id);

	auto const& in_data = svc.data.npc[label];
	dimensions.x = in_data["dimensions"][0].as<float>();
	dimensions.y = in_data["dimensions"][1].as<float>();
	sprite_dimensions.x = in_data["sprite_dimensions"][0].as<int>();
	sprite_dimensions.y = in_data["sprite_dimensions"][1].as<int>();
	spritesheet_dimensions.x = in_data["spritesheet_dimensions"][0].as<int>();
	spritesheet_dimensions.y = in_data["spritesheet_dimensions"][1].as<int>();
	sprite_offset.x = in_data["sprite_offset"][0].as<float>();
	sprite_offset.y = in_data["sprite_offset"][1].as<float>();

	sprite.setOrigin(in_data["sprite_origin"][0].as<float>(), in_data["sprite_origin"][1].as<float>());

	collider = shape::Collider(dimensions);
	collider.sync_components();

	collider.physics.set_global_friction(0.99f);
	collider.stats.GRAV = 6.2f;

	drawbox.setFillColor(sf::Color::Transparent);
	drawbox.setOutlineThickness(-1);
	drawbox.setSize(dimensions);

	if (svc.assets.npcs.contains(label)) { sprite.setTexture(svc.assets.npcs.at(label)); }
	direction.lr = dir::LR::left;
}

void NPC::update(automa::ServiceProvider& svc, world::Map& map, gui::Console& console, player::Player& player) {
	direction.lr = (player.collider.physics.position.x < collider.physics.position.x) ? dir::LR::left : dir::LR::right;
	Entity::update(svc, map);
	if (abs(collider.physics.velocity.x) > physical.walk_threshold) { animation_machine->animation_flags.set(NPCAnimState::walk); }
	if (ent_state.test(entity::State::flip)) { animation_machine->animation_flags.set(NPCAnimState::turn); }
	animation_machine->update();

	if (animation_machine->communication_flags.test(NPCCommunication::sprite_flip)) {
		sprite_flip();
		animation_machine->communication_flags.reset(NPCCommunication::sprite_flip);
	}

	collider.update(svc);
	collider.detect_map_collision(map);
	collider.reset();
	collider.physics.acceleration = {};

	if (player.collider.bounding_box.overlaps(collider.bounding_box) || (triggers.test(NPCTrigger::distant_interact) && state_flags.test(NPCState::force_interact))) {
		state_flags.set(NPCState::engaged);
		if ((player.controller.inspecting() || state_flags.test(NPCState::force_interact)) && !conversations.empty()) {
			state_flags.set(NPCState::introduced);
			console.set_source(svc.text.npc);
			std::string name = std::string(label);
			std::string convo = std::string(conversations.front());
			std::string target = name + convo;
			console.load_and_launch(target);
			console.include_portrait(id);
		}
	} else {
		state_flags.reset(NPCState::engaged);
	}

	if (console.off() && state_flags.test(NPCState::engaged)) {
		if (conversations.size() > 1) {
			conversations.pop_front();
		}
	}

	triggers = {};
}

void NPC::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> campos) {
	sprite.setPosition(collider.physics.position.x - campos.x + sprite_offset.x, collider.physics.position.y - campos.y + sprite_offset.y);
	if (spritesheet_dimensions.y > 0) {
		int u = (int)(animation_machine->animation.get_frame() / spritesheet_dimensions.y) * sprite_dimensions.x;
		int v = (int)(animation_machine->animation.get_frame() % spritesheet_dimensions.y) * sprite_dimensions.y;
		sprite.setTextureRect(sf::IntRect({u, v}, {(int)sprite_dimensions.x, (int)sprite_dimensions.y}));
	}

	if (svc.greyblock_mode()) {
		collider.render(win, campos);
		drawbox.setPosition(collider.physics.position - campos);
		state_flags.test(NPCState::engaged) ? drawbox.setFillColor(svc.styles.colors.green) : drawbox.setFillColor(svc.styles.colors.dark_orange);
		win.draw(drawbox);
	} else {
		win.draw(sprite);
	}
}

void NPC::set_position(sf::Vector2<float> pos) { collider.physics.position = pos; }

void NPC::set_position_from_scaled(sf::Vector2<float> scaled_pos) { collider.physics.position = scaled_pos * 32.f; }

void NPC::set_id(int new_id) { id = new_id; }

void NPC::push_conversation(std::string_view convo) { conversations.push_back(convo); }

void NPC::flush_conversations() { conversations.clear(); }

} // namespace npc
