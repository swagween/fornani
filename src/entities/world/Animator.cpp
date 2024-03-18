
#include "Animator.hpp"
#include "../../service/Serviceprovider.hpp"
#include "../player/Player.hpp"

namespace entity {

Animator::Animator(automa::ServiceProvider& svc, Vecu16 dim, Vecu16 pos) : scaled_dimensions(dim), scaled_position(pos) {
	animation.set_params(still);
	dimensions = static_cast<Vec>(dim * A_UNIT_SIZE);
	position = static_cast<Vec>(pos * A_UNIT_SIZE);
	bounding_box = shape::Shape(dimensions);
	bounding_box.set_position(position);
	large = scaled_dimensions.x == 2;
	if (large) {
		sprite.setTexture(svc.assets.t_large_animators);
		sprite_dimensions = {64, 64};
	} else {
		sprite.setTexture(svc.assets.t_small_animators);
		sprite_dimensions = {32, 32};
	}
}
void Animator::update(player::Player& player) {

	animation.update();
	if (bounding_box.overlaps(player.collider.bounding_box) && player.controller.moving()) {
		if (!activated) {
			animation.set_params(moving);
			animation.start();
			activated = true;
		}
	} else {
		if (animation.complete() && animation.keyframe_over()) {
			animation.set_params(still);
			activated = false;
		}
	}
	position = static_cast<Vec>(scaled_position * A_UNIT_SIZE);
	Vec adjusted_pos = Vec(scaled_position.x * entity::A_UNIT_SIZE + entity::large_animator_offset.x, scaled_position.y * entity::A_UNIT_SIZE + entity::large_animator_offset.y);
	dimensions = static_cast<Vec>(scaled_dimensions * A_UNIT_SIZE);
	bounding_box.set_position(adjusted_pos);
	int converted_id = large ? id - 100 : id - 200;
	// get UV coords
	int u = converted_id * scaled_dimensions.x * A_UNIT_SIZE;
	int v = get_frame() * dimensions.y;
	sprite.setTextureRect(sf::IntRect({u, v}, {sprite_dimensions.x, sprite_dimensions.y}));
}
void Animator::render(sf::RenderWindow& win, Vec campos) {
	sprite.setPosition((int)(position.x - campos.x), (int)(position.y - campos.y));
	win.draw(sprite);
	
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
	/*win.draw(box);
	*/
}
int Animator::get_frame() const { return animation.get_frame(); }

} // namespace entity