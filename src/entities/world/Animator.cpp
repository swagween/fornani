
#include "Animator.hpp"
#include "../../service/ServiceProvider.hpp"
#include "../player/Player.hpp"

namespace entity {

Animator::Animator(automa::ServiceProvider& svc, sf::Vector2<int> pos, bool large) : scaled_position(pos), large(large) {
	drawbox.setFillColor(sf::Color::Transparent);
	drawbox.setOutlineColor(svc.styles.colors.dark_orange);
	drawbox.setOutlineThickness(-1);
	spritesheet_dimensions = {1024, 320};
	animation.set_params(still);
	position = static_cast<sf::Vector2<float>>(pos * A_UNIT_SIZE);
	bounding_box.set_position(position);
	large ? sprite.setTexture(svc.assets.t_large_animators) : sprite.setTexture(svc.assets.t_small_animators);
	sprite_dimensions = large ? sf::Vector2<int>{64, 64} : sf::Vector2<int>{32, 32};
	dimensions = large ? sf::Vector2<float>(2.f * A_UNIT_SIZE, 2.f * A_UNIT_SIZE) : sf::Vector2<float>(A_UNIT_SIZE, A_UNIT_SIZE);
	bounding_box = shape::Shape(sf::Vector2<float>(A_UNIT_SIZE, A_UNIT_SIZE));
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
	position = static_cast<sf::Vector2<float>>(scaled_position * A_UNIT_SIZE);
	sf::Vector2<float> adjusted_pos = sf::Vector2<float>(scaled_position.x * A_UNIT_SIZE + large_animator_offset.x, scaled_position.y * A_UNIT_SIZE + large_animator_offset.y);
	bounding_box.set_position(adjusted_pos);
	int converted_id = large ? id - 100 : id - 200;
	auto x_scale = large ? 2 : 1;
	// get UV coords
	int u = converted_id * x_scale * A_UNIT_SIZE;
	int v = get_frame() * dimensions.y;
	sprite.setTextureRect(sf::IntRect({u, v}, {sprite_dimensions.x, sprite_dimensions.y}));
}
void Animator::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam) {
	activated ? drawbox.setOutlineColor(svc.styles.colors.green) : drawbox.setOutlineColor(svc.styles.colors.dark_orange);
	drawbox.setSize(bounding_box.dimensions);
	drawbox.setPosition(position.x - cam.x, position.y - cam.y);
	sprite.setPosition(position.x - cam.x, position.y - cam.y);
	if (svc.debug_flags.test(automa::DebugFlags::greyblock_mode)) {
		win.draw(drawbox);
	} else {
		win.draw(sprite);
	}
}
int Animator::get_frame() const { return animation.get_frame(); }

} // namespace entity