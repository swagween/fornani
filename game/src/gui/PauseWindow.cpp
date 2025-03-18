
#include "fornani/gui/PauseWindow.hpp"
#include "fornani/service/ServiceProvider.hpp"

namespace fornani::gui {

PauseWindow::PauseWindow(automa::ServiceProvider& svc) : m_menu(svc, {"resume", "settings", "controls", "quit"}, svc.window->f_center_screen() + sf::Vector2f{0.f, 32.f}), m_dimensions{0.f, 0.f} {
	m_background.setSize(svc.window->f_screen_dimensions());
	auto color = svc.styles.colors.ui_black;
	color.a = 220;
	m_background.setFillColor(color);
	svc.soundboard.flags.console.set(audio::Console::menu_open);
}

void PauseWindow::update(automa::ServiceProvider& svc, std::optional<std::unique_ptr<Console>>& console) {
	m_menu.update(svc, m_dimensions, svc.window->f_center_screen());
	if (console) { return; }
	if (svc.controller_map.digital_action_status(config::DigitalAction::menu_down).triggered) { m_menu.down(svc); }
	if (svc.controller_map.digital_action_status(config::DigitalAction::menu_up).triggered) { m_menu.up(svc); }
	if (svc.controller_map.digital_action_status(config::DigitalAction::menu_select).triggered) {
		switch (m_menu.get_selection()) {
		case 0:
			m_state = PauseWindowState::exit;
			svc.soundboard.flags.menu.set(audio::Menu::backward_switch);
			break;
		case 1:
			m_state = PauseWindowState::settings;
			svc.soundboard.flags.menu.set(audio::Menu::forward_switch);
			break;
		case 2:
			m_state = PauseWindowState::controls;
			svc.soundboard.flags.menu.set(audio::Menu::forward_switch);
			break;
		case 3:
			console = std::make_unique<Console>(svc, svc.text.basic, "menu_return", OutputType::gradual);
			svc.soundboard.flags.menu.set(audio::Menu::forward_switch);
			break;
		}
	}
}

void PauseWindow::render(automa::ServiceProvider& svc, sf::RenderWindow& win) {
	win.draw(m_background);
	m_menu.render(win, false);
}

void gui::PauseWindow::reset() { m_state = PauseWindowState::active; }

} // namespace fornani::gui
