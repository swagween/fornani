
#include "SavePoint.hpp"
#include "../../setup/ServiceLocator.hpp"
#include "../../service/ServiceProvider.hpp"

namespace entity {

SavePoint::SavePoint() {
	id = -1;
	dimensions = {32, 32};
	bounding_box = shape::Shape(dimensions);
	proximity_box = shape::Shape(dimensions * 16.f);

	animation.set_params(anim_params);
	sprite.setTexture(svc::assetLocator.get().savepoint);

	sparkle_behavior = {50, 3, 4.3, 0.4, 0.8, -0.3, 0.0, 0.97, 0.97};
	sparkle_stats = {10, 0, 80, 30};
	sparkles = vfx::Emitter(sparkle_behavior, sparkle_stats, flcolor::goldenrod);
	sparkles.set_position(scaled_position.x * 32.f, scaled_position.y * 32.f);
}

void SavePoint::update(automa::ServiceProvider& svc) {

	animation.update();
	sparkles.update();

	sf::Vector2<float> proximity_offset = proximity_box.dimensions * 0.5f + dimensions * 0.5f;
	position = static_cast<Vec>(scaled_position) * 32.f;
	sparkles.set_position(position.x + dimensions.x * 0.5f, position.y + dimensions.y);
	bounding_box.set_position(position);
	proximity_box.set_position(position - proximity_offset);
	activated = false;

	if (svc::playerLocator.get().collider.bounding_box.SAT(proximity_box)) {
		svc::soundboardLocator.get().proximities.save = abs(svc::playerLocator.get().collider.bounding_box.position.x - bounding_box.position.x);

		if (svc::playerLocator.get().collider.bounding_box.SAT(bounding_box)) {
			intensity = 3;
			if (animation.keyframe_over()) { animation.params.framerate = 16; }
			if (svc::playerLocator.get().controller.inspecting()) {
				if (can_activate) {
					activated = true;
					save();
					svc::soundboardLocator.get().world.set(audio::World::soft_sparkle);
					svc::consoleLocator.get().load_and_launch(svc, "save");
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

void SavePoint::render(sf::RenderWindow& win, Vec campos) {

	sparkles.render(win, campos);

	sprite.setPosition((int)(position.x - 16.f - campos.x), (int)(position.y - 32.f - campos.y));
	// get UV coords (only one row of sprites is supported)
	int u = intensity * sprite_dimensions.x;
	int v = (int)(animation.get_frame() * sprite_dimensions.y);
	sprite.setTextureRect(sf::IntRect({u, v}, {(int)sprite_dimensions.x, (int)sprite_dimensions.y}));

	win.draw(sprite);
	svc::counterLocator.get().at(svc::draw_calls)++;
	sf::RectangleShape box{};
	if (activated) {
		box.setFillColor(sf::Color{80, 180, 120, 100});
	} else {
		box.setFillColor(sf::Color{180, 120, 80, 100});
	}
	box.setOutlineColor(sf::Color::White);
	box.setOutlineThickness(-1);
	box.setPosition(bounding_box.position - campos);
	box.setSize(bounding_box.dimensions);
	// win.draw(box);
	box.setPosition(proximity_box.position - campos);
	box.setSize(proximity_box.dimensions);
	// win.draw(box);
	svc::counterLocator.get().at(svc::draw_calls)++;
}

void SavePoint::save() {

	svc::dataLocator.get().save_progress(id);
	can_activate = false;
}

} // namespace entity
