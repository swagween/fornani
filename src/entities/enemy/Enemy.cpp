#include "Enemy.hpp"
#include "../../service/ServiceProvider.hpp"

namespace enemy {
Enemy::Enemy(automa::ServiceProvider& svc, std::string_view label) : entity::Entity(svc), label(label) {

	auto const& in_data = svc.data.enemy[label];

	drawbox.setOutlineColor(svc.styles.colors.ui_white);
	drawbox.setOutlineThickness(-1);
}

void Enemy::update(automa::ServiceProvider& svc) {}

void Enemy::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam) {
	drawbox.setPosition(collider.physics.position + sprite_offset - cam);
	sprite.setPosition(collider.physics.position + sprite_offset - cam);
	if (svc.debug_flags.test(automa::DebugFlags::greyblock_mode)) {
		win.draw(drawbox);
	} else {
		win.draw(sprite);
	}
}

}