
#include "fornani/gui/PauseWindow.hpp"
#include "fornani/service/ServiceProvider.hpp"

namespace fornani::gui {

PauseWindow::PauseWindow(automa::ServiceProvider& svc)
	: PauseWindow{svc, std::vector<std::string>{svc.data.gui_text["pause_menu"]["resume"].as_string(), svc.data.gui_text["pause_menu"]["settings"].as_string(), svc.data.gui_text["pause_menu"]["controls"].as_string(),
												svc.data.gui_text["pause_menu"]["quit"].as_string()}} {}

PauseWindow::PauseWindow(automa::ServiceProvider& svc, std::vector<std::string> options) : m_menu(svc, options, svc.window->f_center_screen() + sf::Vector2f{0.f, 0.f}, "mini_classic"), m_dimensions{} {
	m_background.setSize(svc.window->f_screen_dimensions());
	auto color = colors::ui_black;
	color.a = 220;
	m_background.setFillColor(color);
	svc.soundboard.flags.console.set(audio::Console::menu_open);
	svc.input_system.set_action_set(input::ActionSet::Menu);
}

void PauseWindow::update(automa::ServiceProvider& svc, std::optional<std::unique_ptr<Console>>& console) {
	m_menu.update(svc, m_dimensions, svc.window->f_center_screen());
	if (console) { return; }
	m_menu.handle_inputs(svc.input_system, svc.soundboard);
	if (svc.input_system.digital(input::DigitalAction::menu_select).triggered) {
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
		case 4:
			svc.soundboard.flags.menu.set(audio::Menu::select);
			svc.state_controller.actions.set(automa::Actions::restart);
			break;
		}
	}
}

void PauseWindow::render(automa::ServiceProvider& svc, sf::RenderWindow& win) {
	win.draw(m_background);
	m_menu.render(win, false);
}

void PauseWindow::reset() { m_state = PauseWindowState::active; }

} // namespace fornani::gui
