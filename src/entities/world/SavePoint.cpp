
#include "SavePoint.hpp"
#include "../../setup/ServiceLocator.hpp"
#include "../../service/ServiceProvider.hpp"
#include "../player/Player.hpp"
#include "../../gui/Console.hpp"

namespace entity {

SavePoint::SavePoint(automa::ServiceProvider& svc) {
	id = -1;
	dimensions = {32, 32};
	bounding_box = shape::Shape(dimensions);
	proximity_box = shape::Shape(dimensions * 16.f);
	drawbox.setFillColor(sf::Color::Transparent);
	drawbox.setOutlineThickness(-1);
	drawbox.setSize(dimensions);

	animation.set_params(anim_params);
	sprite.setTexture(svc.assets.savepoint);

	sparkler = vfx::Sparkler(svc, dimensions, svc.styles.colors.green, "save_point");
	sparkler.set_position(position);
}

void SavePoint::update(automa::ServiceProvider& svc, player::Player& player, gui::Console& console) {

	animation.update();
	sparkler.update(svc);

	sf::Vector2<float> proximity_offset = proximity_box.dimensions * 0.5f + dimensions * 0.5f;
	position = static_cast<Vec>(scaled_position) * 32.f;
	sparkler.set_position(position);
	bounding_box.set_position(position);
	proximity_box.set_position(position - proximity_offset);
	activated = false;

	if (player.collider.bounding_box.SAT(proximity_box)) {
		svc::soundboardLocator.get().proximities.save = abs(player.collider.bounding_box.position.x - bounding_box.position.x);

		if (player.collider.bounding_box.SAT(bounding_box)) {
			intensity = 3;
			if (animation.keyframe_over()) { animation.params.framerate = 16; }
			if (player.controller.inspecting()) {
				if (can_activate) {
					activated = true;
					save(svc, player);
					svc::soundboardLocator.get().flags.world.set(audio::World::soft_sparkle);
					console.load_and_launch("save");
				}
			}
		} else {
			intensity = 2;
			if (animation.keyframe_over()) { animation.params.framerate = 20; }
			can_activate = true;
		}
	} else {
		intensity = 1;
		if (animation.keyframe_over()) { animation.params.framerate = 24; }
	}
}

void SavePoint::render(automa::ServiceProvider& svc, sf::RenderWindow& win, Vec campos) {

	sparkler.render(svc, win, campos);

	sprite.setPosition((int)(position.x - 16.f - campos.x), (int)(position.y - 32.f - campos.y));
	// get UV coords (only one row of sprites is supported)
	int u = intensity * sprite_dimensions.x;
	int v = (int)(animation.get_frame() * sprite_dimensions.y);
	sprite.setTextureRect(sf::IntRect({u, v}, {(int)sprite_dimensions.x, (int)sprite_dimensions.y}));

	if (svc.debug_flags.test(automa::DebugFlags::greyblock_mode)) {
		drawbox.setPosition(position - campos);
		activated ? drawbox.setOutlineColor(svc.styles.colors.green) : drawbox.setOutlineColor(svc.styles.colors.dark_orange);
		win.draw(drawbox);
	} else {
		win.draw(sprite);
	}
}

void SavePoint::save(automa::ServiceProvider& svc, player::Player& player) {

	svc.data.save_progress(player, id);
	can_activate = false;
}

} // namespace entity
