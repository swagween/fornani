
#include <fornani/gui/HealthBar.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/utils/Math.hpp>

namespace fornani::gui {

HealthBar::HealthBar(automa::ServiceProvider& svc, sf::Color color) : p_bar_size{48.f, 6.f} {
	p_current.setFillColor(color);
	p_taken.setFillColor(colors::navy_blue);
	p_backdrop.setFillColor(colors::ui_black);

	p_current.setSize(p_bar_size);
	p_taken.setSize(p_bar_size);
	p_backdrop.setSize(p_bar_size);
}

void HealthBar::update(float const percentage, sf::Vector2f position) {
	static float previous_percentage{};
	auto rounded = std::floor(percentage * p_bar_size.x / constants::f_scale_factor) * constants::f_scale_factor / p_bar_size.x;
	p_current.setScale(sf::Vector2f{rounded, 1.f});
	p_steering.seek(sf::Vector2f{percentage, 1.f}, 0.0004f);
	auto taken_target = sf::Vector2f{std::clamp(p_steering.physics.position.x, 0.f, 1.f), 1.f};
	taken_target.x = std::floor(taken_target.x * p_bar_size.x / constants::f_scale_factor) * constants::f_scale_factor / p_bar_size.x; // pixel accuracy
	p_taken.setScale(taken_target);
	previous_percentage = percentage;
	m_world_position = position;
}

void HealthBar::render(sf::RenderWindow& win, sf::Vector2f cam, bool window_fixed) {
	if (!window_fixed) {
		p_current.setPosition(m_world_position - cam);
		p_taken.setPosition(m_world_position - cam);
		p_backdrop.setPosition(m_world_position - cam);
	}
	win.draw(p_backdrop);
	win.draw(p_taken);
	win.draw(p_current);
}

} // namespace fornani::gui
