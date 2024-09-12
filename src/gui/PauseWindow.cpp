#include "PauseWindow.hpp"
#include "../entities/player/Player.hpp"
#include "../service/ServiceProvider.hpp"

namespace gui {

PauseWindow::PauseWindow(automa::ServiceProvider& svc) : Console::Console(svc), selector(svc, {2, 1}), menu(svc, {"resume", "quit"}) {
	title.setString("PAUSED");
	title.setCharacterSize(ui.title_size);
	title_font.loadFromFile(svc.text.title_font);
	title_font.setSmooth(false);
	title.setFont(title_font);
	title.setFillColor(svc.styles.colors.ui_white);
	title.setLetterSpacing(2.f);

	widget_font.loadFromFile(svc.text.text_font);
	widget_font.setSmooth(false);
	widget_label.setString("Press [Enter] to return to Main Menu.");
	widget_label.setCharacterSize(ui.widget_size);
	widget_label.setFont(widget_font);
	widget_label.setFillColor(svc.styles.colors.ui_white);

	help_marker.init(svc, "Press [", config::DigitalAction::platformer_toggle_pause, "] to resume game.", 20, true);
	help_marker.set_position({static_cast<float>(svc.constants.screen_dimensions.x) * 0.5f, static_cast<float>(svc.constants.screen_dimensions.y) - 30.f});

	origin = svc.constants.f_center_screen;
	title.setOrigin(title.getLocalBounds().getSize() * 0.5f);
	title.setPosition(origin + ui.title_offset);
	widget_label.setPosition(origin + ui.widget_label_offset);
	dimensions = {180.f, 180.f};

	position = svc.constants.f_center_screen;
	flags.reset(ConsoleFlags::portrait_included);

	sprite.set_position(svc.constants.f_center_screen);
	sprite.set_force(1.2f);
	sprite.set_fric(0.90f);
	menu.set_force(1.2f);
	menu.set_fric(0.90f);
}

void PauseWindow::update(automa::ServiceProvider& svc, Console& console, bool automatic) {
	Console::update(svc);
	menu.update(svc, {128.f, 128.f}, {svc.constants.f_center_screen.x, svc.constants.f_center_screen.y + 32.f});
	selector.update();
	if (active()) {
		if (svc.controller_map.digital_action_status(config::DigitalAction::menu_down).triggered) {
			menu.down(svc);
		}
		if (svc.controller_map.digital_action_status(config::DigitalAction::menu_up).triggered) {
			menu.up(svc);
		}
		if (svc.controller_map.digital_action_status(config::DigitalAction::menu_select).triggered) {
			switch (menu.get_selection()) {
			case 0:
				flags.set(ConsoleFlags::exited);
				close();
				svc.soundboard.flags.menu.set(audio::Menu::backward_switch);
				break;
			case 1:
				if (automatic) {
					m_services->state_controller.actions.set(automa::Actions::main_menu);
					console.end();
				} else {
					console.set_source(svc.text.basic);
					console.load_and_launch("menu_return");
				}
				flags.set(ConsoleFlags::exited);
				close();
				break;
			}
		}
	}
}

void PauseWindow::render_update(automa::ServiceProvider& svc) {
}

void PauseWindow::render(automa::ServiceProvider& svc, player::Player& player, sf::RenderWindow& win) {
	if (!active()) { return; }
	Console::render(win);
	win.draw(title);
	// win.draw(widget_label);
	// help_marker.render(win);
	menu.render(win, false);
}

void PauseWindow::open() {
	Console::begin();
	menu.open(*m_services, m_services->constants.f_center_screen);
}

void PauseWindow::close() {
	Console::end();
	menu.close(*m_services);
}

} // namespace gui
