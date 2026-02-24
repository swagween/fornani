
#include <fornani/gui/BossHealth.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/utils/Math.hpp>

namespace fornani::gui {

BossHealth::BossHealth(automa::ServiceProvider& svc, std::string_view label)
	: HealthBar{svc, colors::red}, m_label{svc.text.fonts.title}, m_path{svc.finder, std::filesystem::path{"/data/gui/console_paths.json"}, "boss_health", 128, util::InterpolationType::quadratic} {
	m_label.setString(label.data());
	m_label.setCharacterSize(16);
	m_label.setFillColor(colors::red);

	p_taken.setFillColor(colors::goldenrod);
	p_backdrop.setFillColor(colors::ui_black);
	p_backdrop.setOutlineThickness(-2.f);
	p_backdrop.setOutlineColor(colors::red);

	auto const border_v = 32.f;
	auto const label_buffer_v = 4.f;
	auto const bar_height_v = 18.f;

	auto label_offset = sf::Vector2f{m_label.getLocalBounds().size.x + label_buffer_v * 2.f, 0.f};
	auto scale_up = label_buffer_v * 2.f;

	p_current.setSize(sf::Vector2f{svc.window->f_screen_dimensions().x - border_v * 2.f, bar_height_v} - label_offset);
	p_taken.setSize(sf::Vector2f{svc.window->f_screen_dimensions().x - border_v * 2.f, bar_height_v} - label_offset);
	p_backdrop.setSize(sf::Vector2f{svc.window->f_screen_dimensions().x - border_v * 2.f + scale_up, bar_height_v + scale_up});

	p_bar_size = sf::Vector2f{svc.window->f_screen_dimensions().x - border_v * 2.f, bar_height_v} - label_offset;

	m_label.setPosition({border_v, border_v});

	m_path.set_section("start");
}

void BossHealth::update(float const percentage) {
	HealthBar::update(percentage, {});

	auto const border_v = 32.f;
	auto const label_buffer_v = 4.f;
	auto const bar_height_v = 18.f;

	auto label_offset = sf::Vector2f{m_label.getLocalBounds().size.x + label_buffer_v * 2.f, 0.f};

	p_backdrop.setPosition(m_path.get_position() + sf::Vector2f{border_v - label_buffer_v, border_v - label_buffer_v});
	p_taken.setPosition(m_path.get_position() + sf::Vector2f{border_v, border_v} + label_offset);
	p_current.setPosition(m_path.get_position() + sf::Vector2f{border_v, border_v} + label_offset);

	m_label.setPosition(m_path.get_position() + sf::Vector2f{border_v, border_v});
	m_path.update();
}

void BossHealth::render(sf::RenderWindow& win) {
	HealthBar::render(win, {}, true);
	win.draw(m_label);
}

} // namespace fornani::gui
