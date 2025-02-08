
#include "fornani/entities/world/SavePoint.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/entities/player/Player.hpp"
#include "fornani/gui/Console.hpp"

namespace fornani::entity {

SavePoint::SavePoint(automa::ServiceProvider& svc) : sprite{svc.assets.savepoint} {
	id = -1;
	dimensions = {32, 32};
	bounding_box = shape::Shape(dimensions);
	proximity_box = shape::Shape(dimensions * 16.f);
	drawbox.setFillColor(sf::Color::Transparent);
	drawbox.setOutlineThickness(-1);
	drawbox.setSize(dimensions);

	animation.set_params(anim_params);

	sparkler = vfx::Sparkler(svc, dimensions, svc.styles.colors.green, "save_point");
	sparkler.set_position(position);
}

void SavePoint::update(automa::ServiceProvider& svc, player::Player& player, gui::Console& console) {

	animation.update();
	sparkler.update(svc);
	intensity < 2 ? sparkler.set_color(svc.styles.colors.periwinkle) : sparkler.set_color(svc.styles.colors.ui_white);

	sf::Vector2<float> proximity_offset = proximity_box.get_dimensions() * 0.5f + dimensions * 0.5f;
	position = static_cast<Vec>(scaled_position) * 32.f;
	sparkler.set_position(position);
	bounding_box.set_position(position);
	proximity_box.set_position(position - proximity_offset);
	activated = false;

	if (player.collider.bounding_box.SAT(proximity_box)) {
		if (player.collider.bounding_box.SAT(bounding_box)) {
			intensity = 3;
			if (animation.keyframe_over()) { animation.params.framerate = 4; }
			if (player.controller.inspecting()) {
				if (can_activate) {
					activated = true;
					save(svc, player);
					svc.state_controller.save_point_id = id;
					svc.soundboard.flags.world.set(audio::World::soft_sparkle);
					console.set_source(svc.text.basic);
					console.load_and_launch("save");
				}
			}
		} else {
			intensity = 2;
			if (animation.keyframe_over()) { animation.params.framerate = 8; }
			can_activate = true;
		}
	} else {
		intensity = 1;
		if (animation.keyframe_over()) { animation.params.framerate = 12; }
	}
}

void SavePoint::render(automa::ServiceProvider& svc, sf::RenderWindow& win, Vec campos) {

	sparkler.render(svc, win, campos);

	auto offset = sf::Vector2<float>{16.f, 32.f};
	sprite.setPosition(position - offset - campos);

	int u = static_cast<int>(intensity) * static_cast<int>(sprite_dimensions.x);
	int v = static_cast<int>(animation.get_frame() * sprite_dimensions.y);
	sprite.setTextureRect(sf::IntRect({u, v}, {static_cast<int>(sprite_dimensions.x), static_cast<int>(sprite_dimensions.y)}));

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
