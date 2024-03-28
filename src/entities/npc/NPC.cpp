
#include "NPC.hpp"
#include "../../gui/Console.hpp"
#include "../../service/ServiceProvider.hpp"
#include "../player/Player.hpp"

namespace entity {

NPC::NPC(automa::ServiceProvider& svc, int id) : id(id) {

	label = svc.tables.npc_label.at(id);
	conversation = "_01";

	auto const& in_data = svc.data.npc[label];
	dimensions.x = in_data["dimensions"][0].as<float>();
	dimensions.y = in_data["dimensions"][1].as<float>();
	sprite_dimensions.x = in_data["sprite_dimensions"][0].as<int>();
	sprite_dimensions.y = in_data["sprite_dimensions"][1].as<int>();
	spritesheet_dimensions.x = in_data["spritesheet_dimensions"][0].as<int>();
	spritesheet_dimensions.y = in_data["spritesheet_dimensions"][1].as<int>();
	sprite_offset.x = in_data["sprite_offset"][0].as<int>();
	sprite_offset.y = in_data["sprite_offset"][1].as<int>();

	sprite.setOrigin(in_data["sprite_origin"][0].as<float>(), in_data["sprite_origin"][1].as<float>());

	animation.NPC_idle.lookup = in_data["animation"]["idle"][0].as<int>();
	animation.NPC_idle.duration = in_data["animation"]["idle"][1].as<int>();
	animation.NPC_idle.framerate = in_data["animation"]["idle"][2].as<int>();
	animation.NPC_idle.num_loops = in_data["animation"]["idle"][3].as<int>();
	animation.NPC_idle.repeat_last_frame = (bool)in_data["animation"]["idle"][4].as_bool();

	collider = shape::Collider(dimensions);
	collider.sync_components();

	collider.physics.set_global_friction(0.97f);
	collider.stats.GRAV = 4.2f;

	drawbox.setFillColor(sf::Color::Transparent);
	drawbox.setOutlineThickness(-1);
	drawbox.setSize(dimensions);

	sprite.setTexture(svc.assets.npcs.at(label));
	animation.animation.set_params(animation.NPC_idle);
	direction.lr = dir::LR::left;
}

void NPC::update(automa::ServiceProvider& svc, world::Map& map, gui::Console& console, player::Player& player) {
	animation.update();

	direction.lr = (player.collider.physics.position.x < collider.physics.position.x) ? dir::LR::left : dir::LR::right;
	Entity::update(svc, map);
	if (ent_state.test(entity::State::flip)) {
		animation.state.set(NPCAnimState::turn);
		sprite_flip();
	}

	collider.update(svc);
	collider.detect_map_collision(map);
	collider.reset();
	collider.physics.acceleration = {};

	state_flags.reset(NPCState::engaged);
	if (player.collider.bounding_box.overlaps(collider.bounding_box)) {
		if (player.controller.inspecting()) {
			state_flags.set(NPCState::engaged);
			console.set_source(svc.text.npc);
			std::string name = std::string(label);
			std::string convo = std::string(conversation);
			std::string target = name + convo;
			console.load_and_launch(target);
			console.include_portrait(id);
		}
	}
}

void NPC::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> campos) {
	sprite.setPosition(collider.physics.position.x - campos.x + sprite_offset.x, collider.physics.position.y - campos.y + sprite_offset.y);
	int u = (int)(animation.get_frame() / spritesheet_dimensions.y) * sprite_dimensions.x;
	int v = (int)(animation.get_frame() % spritesheet_dimensions.y) * sprite_dimensions.y;
	sprite.setTextureRect(sf::IntRect({u, v}, {(int)sprite_dimensions.x, (int)sprite_dimensions.y}));

	if (svc.greyblock_mode()) {
		drawbox.setPosition(collider.physics.position - campos);
		state_flags.test(NPCState::engaged) ? drawbox.setOutlineColor(svc.styles.colors.green) : drawbox.setOutlineColor(svc.styles.colors.dark_orange);
		win.draw(drawbox);
		collider.render(win, campos);
	} else {
		win.draw(sprite);
	}
}

void NPC::set_position(sf::Vector2<float> pos) { collider.physics.position = pos; }

void NPC::set_position_from_scaled(sf::Vector2<float> scaled_pos) { collider.physics.position = scaled_pos * 32.f; }

void NPC::set_id(int new_id) { id = new_id; }

} // namespace entity
