
#include "fornani/entities/world/Chest.hpp"
#include "fornani/entities/player/Player.hpp"
#include "fornani/gui/Console.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/world/Map.hpp"

namespace fornani::entity {

Chest::Chest(automa::ServiceProvider& svc, int id) : id(id), sprite{svc.assets.get_texture("chest")} {
	dimensions = {28, 28};
	sprite_dimensions = {32, 28};
	spritesheet_dimensions = {224, 28};
	collider = shape::Collider(dimensions);
	collider.sync_components();
	collider.physics.elasticity = 0.1f;

	collider.physics.set_global_friction(0.999f);
	collider.stats.GRAV = 6.2f;

	drawbox.setFillColor(sf::Color::Transparent);
	drawbox.setOutlineThickness(-1);
	drawbox.setSize(dimensions);

	animation.set_params(unopened);

	sparkler = vfx::Sparkler(svc, dimensions, svc.styles.colors.ui_white, "chest");
	sparkler.set_position(collider.physics.position);
	if (svc.data.chest_is_open(id)) { state.set(ChestState::open); }
}

void Chest::update(automa::ServiceProvider& svc, world::Map& map, gui::Console& console, player::Player& player) {
	animation.update();
	if (!state.test(ChestState::open)) {
		if (svc.ticker.every_x_ticks(1200)) { animation.set_params(shine); }
		if (animation.complete() && !state.test(ChestState::activated)) { animation.set_params(unopened); }
	} else {
		animation.set_params(opened);
	}
	sparkler.update(svc);

	collider.update(svc);
	for (auto& breakable : map.breakables) { collider.handle_collider_collision(breakable.collider); }
	for (auto& pushable : map.pushables) { collider.handle_collider_collision(pushable.collider); }
	for (auto& platform : map.platforms) { collider.handle_collider_collision(platform); }
	for (auto& button : map.switch_buttons) { collider.handle_collider_collision(button->collider); }
	for (auto& block : map.switch_blocks) {
		if (block.on()) { collider.handle_collider_collision(block.collider); }
	}
	collider.detect_map_collision(map);
	collider.reset();
	collider.reset_ground_flags();
	collider.physics.acceleration = {};

	sparkler.set_position(collider.physics.position);
	state.reset(ChestState::activated);

	if (player.collider.bounding_box.overlaps(collider.bounding_box)) {
		if (player.controller.inspecting()) {
			state.set(ChestState::activated);
			console.set_source(svc.text.basic);
			if (!state.test(ChestState::open)) {
				svc.soundboard.flags.world.set(audio::World::chest);
				state.set(ChestState::open);
				animation.set_params(opened);
				svc.data.open_chest(id);
				if (type == ChestType::gun) {
					player.push_to_loadout(item_id);
					console.display_gun(item_id);
					console.load_and_launch("chest");
					console.append(player.arsenal.value().get_weapon_at(item_id).get_label());
				}
				if (type == ChestType::orbs) { map.active_loot.push_back(item::Loot(svc, {loot.amount, loot.amount}, loot.rarity, collider.bounding_box.get_position(), 100)); }
				if (type == ChestType::item) {
					player.give_item(m_item, item::ItemType::key, 1);
					console.display_item(item_id);
					console.load_and_launch("chest");
					console.append(player.catalog.inventory.item_view(item_id).get_label());
				}
			} else {
				console.load_and_launch("open_chest");
			}
		}
	}
}

void Chest::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> campos) {
	sprite.setPosition({collider.physics.position.x - campos.x, collider.physics.position.y - campos.y + 1});
	// get UV coords (only one row of sprites is supported)
	int u = (int)(animation.get_frame() * sprite_dimensions.x);
	int v = 0;
	sprite.setTextureRect(sf::IntRect({u, v}, {static_cast<int>(sprite_dimensions.x), static_cast<int>(sprite_dimensions.y)}));

	if (svc.debug_flags.test(automa::DebugFlags::greyblock_mode)) {
		drawbox.setPosition(collider.physics.position - campos);
		state.test(ChestState::activated) ? drawbox.setOutlineColor(svc.styles.colors.green) : drawbox.setOutlineColor(svc.styles.colors.dark_orange);
		win.draw(drawbox);
		collider.render(win, campos);
	} else {
		win.draw(sprite);
		// sparkler.render(svc, win, campos);
	}
}

void Chest::set_position(sf::Vector2<float> pos) { collider.physics.position = pos; }

void Chest::set_position_from_scaled(sf::Vector2<float> scaled_pos) { collider.physics.position = scaled_pos * 32.f; }

void Chest::set_id(int new_id) { id = new_id; }

void Chest::set_item(int to_id) { item_id = to_id; }

void Chest::set_amount(int to_amount) { loot.amount = to_amount; }

void Chest::set_rarity(float to_rarity) { loot.rarity = to_rarity; }

void Chest::set_type(ChestType to_type) { type = to_type; }

} // namespace fornani::entity
