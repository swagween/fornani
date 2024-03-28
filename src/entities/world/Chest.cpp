
#include "Chest.hpp"
#include "../../gui/Console.hpp"
#include "../../service/ServiceProvider.hpp"
#include "../player/Player.hpp"

namespace entity {

Chest::Chest(automa::ServiceProvider& svc) {
	dimensions = {32, 28};
	sprite_dimensions = {32, 28};
	spritesheet_dimensions = {224, 28};
	collider = shape::Collider(dimensions);
	collider.sync_components();
	collider.physics.elasticity = 1.0f;

	collider.physics.set_global_friction(0.99f);
	collider.stats.GRAV = 4.2f;

	drawbox.setFillColor(sf::Color::Transparent);
	drawbox.setOutlineThickness(-1);
	drawbox.setSize(dimensions);

	animation.set_params(unopened);
	sprite.setTexture(svc.assets.t_chest);

	sparkler = vfx::Sparkler(svc, dimensions, svc.styles.colors.ui_white, "chest");
	sparkler.set_position(collider.physics.position);
}

void Chest::update(automa::ServiceProvider& svc, world::Map& map, gui::Console& console, player::Player& player) {

	if (!open) {
		if (svc.ticker.every_x_frames(400)) { animation.set_params(shine); }
		if (animation.complete() && !activated) { animation.set_params(unopened); }
	} else {
		animation.set_params(opened); 
	}

	animation.update();
	sparkler.update(svc);

	collider.update(svc);
	collider.detect_map_collision(map);
	collider.reset();
	collider.reset_ground_flags();
	collider.physics.acceleration = {};

	sparkler.set_position(collider.physics.position);
	activated = false;

	if (player.collider.bounding_box.overlaps(collider.bounding_box)) {
		if (player.controller.inspecting()) {
			activated = true;
			console.set_source(svc.text.basic);
			if (!open) {
				svc.soundboard.flags.world.set(audio::World::chest);
				console.load_and_launch("chest");
				open = true;
				animation.set_params(opened);
			} else {
				console.load_and_launch("open_chest");
			}
		}
	}
}

void Chest::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> campos) {
	sprite.setPosition(collider.physics.position.x - campos.x, collider.physics.position.y - campos.y + 1);
	// get UV coords (only one row of sprites is supported)
	int u = (int)(animation.get_frame() * sprite_dimensions.x);
	int v = 0;
	sprite.setTextureRect(sf::IntRect({u, v}, {(int)sprite_dimensions.x, (int)sprite_dimensions.y}));

	if (svc.debug_flags.test(automa::DebugFlags::greyblock_mode)) {
		drawbox.setPosition(collider.physics.position - campos);
		activated ? drawbox.setOutlineColor(svc.styles.colors.green) : drawbox.setOutlineColor(svc.styles.colors.dark_orange);
		win.draw(drawbox);
		collider.render(win, campos);
	} else {
		win.draw(sprite);
		//sparkler.render(svc, win, campos);
	}
}

void Chest::set_position(sf::Vector2<float> pos) { collider.physics.position = pos; }

void Chest::set_position_from_scaled(sf::Vector2<float> scaled_pos) { collider.physics.position = scaled_pos * 32.f; }

void Chest::set_id(int new_id) { id = id; }

} // namespace entity
