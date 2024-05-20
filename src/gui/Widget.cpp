#include "Widget.hpp"
#include "../entities/player/Player.hpp"
#include "../service/ServiceProvider.hpp"
#include <algorithm>

namespace gui {
Widget::Widget(automa::ServiceProvider& svc, sf::Vector2<int> dim, int index) : dimensions(dim), index(index) {
	gravitator = vfx::Gravitator({0, 0}, svc.styles.colors.bright_orange, 0.9f);
	gravitator.collider.physics = components::PhysicsComponent(sf::Vector2<float>{0.9f, 0.9f}, 1.0f);
}
void Widget::update(automa::ServiceProvider& svc, player::Player& player) {
	gravitator.set_target_position(position);
	gravitator.update(svc);
	maximum = player.health.get_max();
}
void Widget::render(sf::RenderWindow& win) {
	int lookup = static_cast<int>(current_state) * dimensions.x;
	sprite.setTextureRect(sf::IntRect({{lookup, 0}, dimensions}));
	draw_position = gravitator.collider.physics.position;
	auto diff = gravitator.collider.physics.position - position;
	if (abs(diff.x) < 1.f) { draw_position.x = position.x; }
	if (abs(diff.y) < 1.f) { draw_position.y = position.y; }
	sprite.setPosition(origin + draw_position);
	if (index < maximum) { win.draw(sprite); }
}
} // namespace gui
