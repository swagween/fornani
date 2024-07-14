#include "Portrait.hpp"
#include "../service/ServiceProvider.hpp"

namespace gui {

Portrait::Portrait(automa::ServiceProvider& svc, bool left) : is_nani(!left) {
	dimensions = sf::Vector2<float>{128, 256};
	end_position = sf::Vector2<float>{pad_x, svc.constants.screen_dimensions.y - pad_y - dimensions.y};
	bring_in();
	if (is_nani) {
		end_position.x = svc.constants.screen_dimensions.x - pad_x - dimensions.x;
		id = 5; // nani :)
	}
	float fric{0.85f};
	gravitator = vfx::Gravitator(start_position, sf::Color::Transparent, 1.f);
	gravitator.collider.physics = components::PhysicsComponent(sf::Vector2<float>{fric, fric}, 2.0f);
	sprite.setTexture(svc.assets.t_portraits);
	sprite.setTextureRect(sf::IntRect({id * (int)dimensions.x, (emotion - 1) * (int)dimensions.y}, {(int)dimensions.x, (int)dimensions.y}));
	gravitator.set_target_position(position);
}

void Portrait::update(automa::ServiceProvider& svc) {
	gravitator.set_target_position(position);
	gravitator.update(svc);
	sprite.setPosition(gravitator.collider.physics.position);
}

void Portrait::render(sf::RenderWindow& win) {
	sprite.setTextureRect(sf::IntRect({id * (int)dimensions.x, (emotion - 1) * (int)dimensions.y}, {(int)dimensions.x, (int)dimensions.y}));
	win.draw(sprite);
}

void Portrait::reset(automa::ServiceProvider& svc) {
	auto rand_y = svc.random.random_range_float(-60.f, 60.f);
	start_position = {-128.f, position.y};
	if (is_nani) { start_position.x = svc.constants.screen_dimensions.x + 132.f; }
	set_position(start_position);
}

void Portrait::set_position(sf::Vector2<float> pos) {
	sprite.setPosition(pos);
	gravitator.set_position(pos);
}

void Portrait::bring_in() { position = end_position; }

void Portrait::send_out() { position = start_position; }

void Portrait::set_emotion(int new_emotion) {
	emotion = new_emotion;
	set_position(start_position);
}

void Portrait::set_id(int new_id) {
	id = new_id;
	sprite.setTextureRect(sf::IntRect({id * (int)dimensions.x, (emotion - 1) * (int)dimensions.y}, {(int)dimensions.x, (int)dimensions.y}));
}

} // namespace gui
