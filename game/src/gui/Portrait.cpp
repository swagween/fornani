
#include "fornani/gui/Portrait.hpp"
#include "fornani/service/ServiceProvider.hpp"

namespace fornani::gui {

Portrait::Portrait(automa::ServiceProvider& svc, sf::Texture const& texture, int id, bool left) : Portrait(svc, id, left) {
	Drawable::set_texture(texture);

	set_scale({1.f, 1.f});
	flags.set(PortraitFlags::custom);
	set_texture_rect(sf::IntRect({}, sf::Vector2i{dimensions * constants::f_scale_factor}));
}

Portrait::Portrait(automa::ServiceProvider& svc, int id, bool left) : Drawable(svc, "character_portraits"), window{svc.assets.get_texture("portrait_window")}, m_id{id}, m_services{&svc} {
	dimensions = sf::Vector2f{64, 128};
	end_position = sf::Vector2{pad_x, svc.window->i_screen_dimensions().y - pad_y - dimensions.y * constants::f_scale_factor};
	bring_in();
	if (!left) { end_position.x = svc.window->i_screen_dimensions().x - pad_x - dimensions.x * constants::f_scale_factor; }
	left ? flags.reset(PortraitFlags::right) : flags.set(PortraitFlags::right);
	reset(svc);

	if (!m_texture.resize({64u, 512u})) {}
	m_texture.clear(colors::transparent);
	auto reel = sf::Sprite(svc.assets.get_texture("character_portraits"));
	reel.setTextureRect(sf::IntRect(sf::Vector2i{dimensions}.componentWiseMul({m_id, 0}), sf::Vector2i{dimensions} * 4));
	m_texture.draw(reel);
	m_texture.setRepeated(true);
	m_texture.setSmooth(false);
	m_texture.display();
}

void Portrait::update(automa::ServiceProvider& svc) {
	m_steering.target(m_physics, position, 0.005f);
	m_physics.simple_update();
	window.setPosition(m_physics.position);
	Drawable::set_position(m_physics.position);
	if (m_sparkler) {
		m_sparkler->set_position(m_physics.position);
		m_sparkler->update(svc);
	}
}

void Portrait::render(sf::RenderWindow& win, bool shader) {
	auto rect = sf::IntRect(sf::Vector2i{dimensions}.componentWiseMul({m_id, m_emotion}), sf::Vector2i{dimensions});
	if (!flags.test(PortraitFlags::custom)) { set_texture_rect(rect); }
	win.draw(window);
	if (!shader) { win.draw(*this); }
	if (m_sparkler) { m_sparkler->render(win, {}); }
}

void Portrait::reset(automa::ServiceProvider& svc) {
	start_position = {-128.f, position.y};
	if (flags.test(PortraitFlags::right)) { start_position.x = svc.window->i_screen_dimensions().x + 132.f; }
	set_position(start_position);
	m_emotion = 0;
}

void Portrait::set_position(sf::Vector2f pos) {
	window.setPosition(pos);
	Drawable::set_position(pos);
	m_physics.position = pos;
}

void Portrait::bring_in() { position = end_position; }

void Portrait::send_out() { position = start_position; }

void Portrait::set_emotion(int new_emotion) {
	m_emotion = new_emotion;
	Portrait::set_position(start_position);
}

void Portrait::add_sparkler(std::string_view tag) { m_sparkler = vfx::Sparkler(*m_services, dimensions * constants::f_scale_factor, colors::ui_black, tag); }

void Portrait::remove_sparkler() { m_sparkler = {}; }

sf::Sprite Portrait::make_sprite() {
	auto rect = sf::IntRect(sf::Vector2i{dimensions}.componentWiseMul({0, m_emotion}), sf::Vector2i{dimensions});
	auto sprite = sf::Sprite{m_texture.getTexture()};
	sprite.setTextureRect(rect);
	sprite.setScale(get_scale());
	sprite.setPosition(get_window_position());
	return sprite;
}

} // namespace fornani::gui
