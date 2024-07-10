
#include "Animator.hpp"
#include "../../service/ServiceProvider.hpp"
#include "../player/Player.hpp"

namespace entity {

Animator::Animator(automa::ServiceProvider& svc, sf::Vector2<int> pos, int id, bool large, bool automatic, bool foreground, int style) : scaled_position(pos), id(id) {
	drawbox.setFillColor(sf::Color::Transparent);
	drawbox.setOutlineColor(svc.styles.colors.dark_orange);
	drawbox.setOutlineThickness(-1);
	spritesheet_dimensions = {1024, 320};
	animation.set_params(still);
	position = sf::Vector2<float>{static_cast<float>(pos.x) * svc.constants.cell_size, static_cast<float>(pos.y) * svc.constants.cell_size};
	bounding_box.set_position(position);
	large ? sprite.setTexture(svc.assets.t_large_animators) : sprite.setTexture(svc.assets.t_small_animators);
	if (large) { attributes.set(AnimatorAttributes::large); }
	if (automatic) {
		sprite.setTexture(svc.assets.animator_lookup.at(style));
		attributes.set(AnimatorAttributes::automatic);
		animation.set_params(automate);
	}
	sprite_dimensions = large ? sf::Vector2<int>{64, 64} : sf::Vector2<int>{32, 32};
	dimensions = large ? sf::Vector2<float>(2.f * svc.constants.cell_size, 2.f * svc.constants.cell_size) : sf::Vector2<float>(svc.constants.cell_size, svc.constants.cell_size);
	bounding_box = shape::Shape(sf::Vector2<float>(svc.constants.cell_size, svc.constants.cell_size));
}
void Animator::update(automa::ServiceProvider& svc, player::Player& player) {

	animation.update();
	if (!attributes.test(AnimatorAttributes::automatic)) {
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
	}
	position = sf::Vector2<float>{static_cast<float>(scaled_position.x) * svc.constants.cell_size, static_cast<float>(scaled_position.y) * svc.constants.cell_size};
	sf::Vector2<float> adjusted_pos = sf::Vector2<float>((float)scaled_position.x * svc.constants.cell_size + large_animator_offset.x, (float)scaled_position.y * svc.constants.cell_size + large_animator_offset.y);
	bounding_box.set_position(adjusted_pos);
	int converted_id = attributes.test(AnimatorAttributes::large) ? id - 100 : id - 200;
	auto x_scale = attributes.test(AnimatorAttributes::large) ? 2 : 1;
	if (attributes.test(AnimatorAttributes::automatic)) { converted_id = id; }
	// get UV coords
	int u = converted_id * x_scale * svc.constants.cell_size;
	int v = get_frame() * static_cast<int>(dimensions.y);
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