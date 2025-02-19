
#include "fornani/gui/PauseWindow.hpp"
#include "fornani/service/ServiceProvider.hpp"

namespace fornani::gui {

PauseWindow::PauseWindow(automa::ServiceProvider& svc) : m_menu(svc, {"resume", "settings", "controls", "quit"}, svc.constants.f_center_screen + sf::Vector2f{0.f, 32.f}), m_dimensions{120.f, 120.f} {
	m_background.setSize(svc.constants.f_screen_dimensions);
	auto color = svc.styles.colors.ui_black;
	color.a = 180;
	m_background.setFillColor(color);
	m_menu.set_force(1.2f);
	m_menu.set_fric(0.90f);
	svc.soundboard.flags.console.set(audio::Console::menu_open);
}

void PauseWindow::update(automa::ServiceProvider& svc, Console& console) {
	m_menu.update(svc, m_dimensions, svc.constants.f_center_screen);
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
			console.set_source(svc.text.basic);
			console.load_and_launch("menu_return");
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
