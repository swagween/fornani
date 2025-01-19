#include "fornani/gui/Widget.hpp"
#include "fornani/entities/player/Player.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/utils/Math.hpp"
#include <algorithm>

namespace gui {

Widget::Widget(automa::ServiceProvider& svc, sf::Texture& texture, sf::Vector2<int> dim, int index) : dimensions(dim), index(index), sprite{texture} {
	gravitator = vfx::Gravitator({}, svc.styles.colors.bright_orange, 0.9f);
	gravitator.collider.physics = components::PhysicsComponent(sf::Vector2<float>{0.9f, 0.9f}, 1.0f);
}

void Widget::update(automa::ServiceProvider& svc, int max) {
	shaking.update();
	gravitator.set_target_position(position);
	gravitator.update(svc);
	maximum = max;
}

void Widget::render(sf::RenderWindow& win) {
	int lookup = static_cast<int>(current_state) * dimensions.x;
	sprite.setTextureRect(sf::IntRect({{lookup, 0}, dimensions}));
	draw_position = gravitator.position() + sf::Vector2<float>{2.f, 2.f};
	shaking.is_complete() ? sprite.setPosition(origin + position) : sprite.setPosition(origin + draw_position);
	if (index < maximum) { win.draw(sprite); }
}

} // namespace gui
