
#include <fornani/gui/BossHealth.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/utils/Math.hpp>

namespace fornani::gui {

constexpr float border_v = 32.f;
constexpr float label_buffer_v = 4.f;
constexpr float bar_height_v = 18.f;

BossHealth::BossHealth(automa::ServiceProvider& svc, std::string_view label) : m_label{svc.text.fonts.title} {
	m_label.setString(label.data());
	m_label.setCharacterSize(16);
	m_label.setFillColor(colors::red);

	m_current.setFillColor(colors::red);
	m_taken.setFillColor(colors::goldenrod);
	m_backdrop.setFillColor(colors::ui_black);
	m_backdrop.setOutlineThickness(-2.f);
	m_backdrop.setOutlineColor(colors::red);

	auto label_offset = sf::Vector2f{m_label.getLocalBounds().size.x + label_buffer_v * 2.f, 0.f};
	auto scale_up = label_buffer_v * 2.f;

	m_current.setSize(sf::Vector2f{svc.window->f_screen_dimensions().x - border_v * 2.f, bar_height_v} - label_offset);
	m_taken.setSize(sf::Vector2f{svc.window->f_screen_dimensions().x - border_v * 2.f, bar_height_v} - label_offset);
	m_backdrop.setSize(sf::Vector2f{svc.window->f_screen_dimensions().x - border_v * 2.f + scale_up, bar_height_v + scale_up});

	m_bar_size = sf::Vector2f{svc.window->f_screen_dimensions().x - border_v * 2.f, bar_height_v} - label_offset;

	m_backdrop.setPosition(sf::Vector2f{border_v - label_buffer_v, border_v - label_buffer_v});
	m_taken.setPosition(sf::Vector2f{border_v, border_v} + label_offset);
	m_current.setPosition(sf::Vector2f{border_v, border_v} + label_offset);
	m_label.setPosition({border_v, border_v});
}

void BossHealth::update(float const percentage) {
	static float previous_percentage{};
	auto rounded = std::floor(percentage * m_bar_size.x / constants::f_scale_factor) * constants::f_scale_factor / m_bar_size.x;
	m_current.setScale(sf::Vector2f{rounded, 1.f});
	m_steering.seek(m_physics, sf::Vector2f{percentage, 1.f}, 0.0004f);
	m_physics.simple_update();
	auto taken_target = sf::Vector2f{std::clamp(m_physics.position.x, 0.f, 1.f), 1.f};
	taken_target.x = std::floor(taken_target.x * m_bar_size.x / constants::f_scale_factor) * constants::f_scale_factor / m_bar_size.x; // pixel accuracy
	m_taken.setScale(taken_target);
	previous_percentage = percentage;
}

void BossHealth::render(sf::RenderWindow& win) {
	win.draw(m_backdrop);
	win.draw(m_label);
	win.draw(m_taken);
	win.draw(m_current);
}

} // namespace fornani::gui
