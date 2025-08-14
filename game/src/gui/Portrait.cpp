#include "fornani/gui/Portrait.hpp"
#include "fornani/service/ServiceProvider.hpp"

namespace fornani::gui {

Portrait::Portrait(automa::ServiceProvider& svc, bool left) : Drawable(svc, "character_portraits"), is_nani(!left), window{svc.assets.get_texture("portrait_window")} {
	dimensions = sf::Vector2f{64, 128};
	end_position = sf::Vector2{pad_x, svc.window->i_screen_dimensions().y - pad_y - dimensions.y * constants::f_scale_factor};
	bring_in();
	if (is_nani) {
		end_position.x = svc.window->i_screen_dimensions().x - pad_x - dimensions.x * constants::f_scale_factor;
		id = 5; // nani :)
	}
	set_texture_rect(sf::IntRect({id * static_cast<int>(dimensions.x), (emotion - 1) * static_cast<int>(dimensions.y)}, {static_cast<int>(dimensions.x), static_cast<int>(dimensions.y)}));
}

void Portrait::update(automa::ServiceProvider& svc) {
	m_steering.target(m_physics, position, 0.005f);
	m_physics.simple_update();
	window.setPosition(m_physics.position);
	Drawable::set_position(m_physics.position);
}

void Portrait::set_custom_portrait(sf::Sprite const& sp) {
	set_sprite(sp);
	set_origin({});
	flags.set(PortraitFlags::custom);
}

void Portrait::render(sf::RenderWindow& win) {
	if (!flags.test(PortraitFlags::custom)) { set_texture_rect(sf::IntRect({id * static_cast<int>(dimensions.x), (emotion - 1) * static_cast<int>(dimensions.y)}, static_cast<sf::Vector2<int>>(dimensions))); }
	win.draw(window);
	win.draw(*this);
}

void Portrait::reset(automa::ServiceProvider& svc) {
	start_position = {-128.f, position.y};
	if (is_nani) { start_position.x = svc.window->i_screen_dimensions().x + 132.f; }
	set_position(start_position);
	emotion = 1;
}

void Portrait::set_position(sf::Vector2f pos) {
	window.setPosition(pos);
	Drawable::set_position(pos);
	m_physics.position = pos;
}

void Portrait::set_texture(sf::Texture const& texture) {
	Drawable::set_texture(texture);
	set_texture_rect(sf::IntRect{{}, sf::Vector2i{texture.getSize()}});
	flags.set(PortraitFlags::custom);
}

void Portrait::bring_in() { position = end_position; }

void Portrait::send_out() { position = start_position; }

void Portrait::set_emotion(int new_emotion) {
	emotion = new_emotion;
	Portrait::set_position(start_position);
}

void Portrait::set_id(int new_id) {
	id = new_id;
	if (!flags.test(PortraitFlags::custom)) { set_texture_rect(sf::IntRect({id * static_cast<int>(dimensions.x), (emotion - 1) * static_cast<int>(dimensions.y)}, sf::Vector2i{dimensions})); }
}

} // namespace fornani::gui
