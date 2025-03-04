#include "fornani/gui/Portrait.hpp"
#include "fornani/service/ServiceProvider.hpp"

namespace fornani::gui {

Portrait::Portrait(automa::ServiceProvider& svc, bool left) : is_nani(!left), sprite{svc.assets.get_texture("character_portraits")}, window{svc.assets.get_texture("portrait_window")} {
	dimensions = sf::Vector2<float>{128, 256};
	end_position = sf::Vector2{pad_x, svc.constants.screen_dimensions.y - pad_y - dimensions.y};
	bring_in();
	if (is_nani) {
		end_position.x = svc.constants.screen_dimensions.x - pad_x - dimensions.x;
		id = 5; // nani :)
	}
	float constexpr fric{0.85f};
	gravitator = vfx::Gravitator(start_position, sf::Color::Transparent, 1.f);
	gravitator.collider.physics = components::PhysicsComponent(sf::Vector2{fric, fric}, 2.0f);
	sprite.setTextureRect(sf::IntRect({id * static_cast<int>(dimensions.x), (emotion - 1) * static_cast<int>(dimensions.y)}, {static_cast<int>(dimensions.x), static_cast<int>(dimensions.y)}));
	gravitator.set_target_position(position);
}

void Portrait::update(automa::ServiceProvider& svc) {
	gravitator.set_target_position(position);
	gravitator.update(svc);
	window.setPosition(gravitator.position());
	sprite.setPosition(gravitator.position());
}

void Portrait::set_custom_portrait(sf::Sprite const& sp) {
	sprite = sp;
	sprite.setOrigin({});
	flags.set(PortraitFlags::custom);
}

void Portrait::render(sf::RenderWindow& win) {
	if (!flags.test(PortraitFlags::custom)) { sprite.setTextureRect(sf::IntRect({id * static_cast<int>(dimensions.x), (emotion - 1) * static_cast<int>(dimensions.y)}, static_cast<sf::Vector2<int>>(dimensions))); }
	win.draw(window);
	win.draw(sprite);
}

void Portrait::reset(automa::ServiceProvider& svc) {
	start_position = {-128.f, position.y};
	if (is_nani) { start_position.x = svc.constants.screen_dimensions.x + 132.f; }
	set_position(start_position);
	emotion = 1;
}

void Portrait::set_position(sf::Vector2<float> pos) {
	window.setPosition(pos);
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
	if (!flags.test(PortraitFlags::custom)) { sprite.setTextureRect(sf::IntRect({id * static_cast<int>(dimensions.x), (emotion - 1) * (int)dimensions.y}, {(int)dimensions.x, (int)dimensions.y})); }
}

} // namespace fornani::gui
