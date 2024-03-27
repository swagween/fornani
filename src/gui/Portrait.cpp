#include "Portrait.hpp"
#include "Portrait.hpp"
#include "Portrait.hpp"
#include "Portrait.hpp"
#include "../service/ServiceProvider.hpp"

namespace gui {

Portrait::Portrait(automa::ServiceProvider& svc) {
	dimensions = sf::Vector2<float>{128, 256};
	position = sf::Vector2<float>{pad_x, svc.constants.screen_dimensions.y - pad_y - dimensions.y};
	float fric{0.99f};
	start_position = {0.f, position.y - 60.f};
	gravitator = vfx::Gravitator(start_position, sf::Color::Transparent, 1.f);
	gravitator.collider.physics = components::PhysicsComponent(sf::Vector2<float>{fric, fric}, 1.0f);
	sprite.setTexture(svc.assets.t_portraits);
	sprite.setTextureRect(sf::IntRect({id * (int)dimensions.x, 0}, {(int)dimensions.x, (int)dimensions.y}));
	gravitator.set_target_position(position);
}

void Portrait::update(automa::ServiceProvider& svc) {
	gravitator.update(svc);
	sprite.setPosition(gravitator.collider.bounding_box.position);
}

void Portrait::render(sf::RenderWindow& win) { win.draw(sprite); }

void Portrait::reset() {
	//gravitator.set_position(start_position);
	gravitator.set_target_position(position);
}

} // namespace gui
