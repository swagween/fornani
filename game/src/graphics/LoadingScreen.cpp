
#include <fornani/graphics/LoadingScreen.hpp>
#include <fornani/service/ServiceProvider.hpp>

namespace fornani {

LoadingScreen::LoadingScreen(automa::ServiceProvider& svc) : m_bar{svc, {300.f, 6.f}, {colors::ui_white, colors::goldenrod, colors::transparent}}, m_services{&svc}, m_readout{svc.text.fonts.title.font} {
	m_readout.setFillColor(colors::ui_white);
	m_readout.setPosition(svc.window->f_center_screen());
	m_readout.setCharacterSize(16);
}

void LoadingScreen::render(sf::RenderWindow& win, float progress, bool bg) {
	if (bg) {
		auto background = sf::RectangleShape{};
		background.setSize(sf::Vector2f{win.getSize()});
		background.setFillColor(colors::ui_black);
		win.draw(background);
	}

	auto buffer = 20.f;
	m_bar.render(win);
	auto box = sf::RectangleShape{};
	box.setSize(m_bar.get_dimensions() + sf::Vector2f{4.f, 4.f});
	box.setOrigin(box.getLocalBounds().getCenter() - sf::Vector2f{0.f, buffer});
	box.setPosition(m_services->window->f_center_screen());
	box.setFillColor(colors::transparent);
	box.setOutlineColor(colors::ui_white);
	box.setOutlineThickness(2.f);
	win.draw(box);
	m_bar.update(*m_services, box.getGlobalBounds().getCenter(), progress);

	m_readout.setString(std::to_string(static_cast<int>(progress * 100.f)) + "%");
	m_readout.setOrigin(m_readout.getLocalBounds().getCenter() + sf::Vector2f{0.f, buffer});
	win.draw(m_readout);
}

void LoadingScreen::stall(sf::RenderWindow& win) { m_bar.render(win); }

} // namespace fornani
