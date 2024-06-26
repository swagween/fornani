#include "PauseWindow.hpp"
#include "../service/ServiceProvider.hpp"
#include "../entities/player/Player.hpp"

namespace gui {

PauseWindow::PauseWindow(automa::ServiceProvider& svc) : Console::Console(svc), selector(svc, {2, 1}) {
	title.setString("PAUSED");
	title.setCharacterSize(ui.title_size);
	title_font.loadFromFile(svc.text.title_font);
	title_font.setSmooth(false);
	title.setFont(title_font);
	title.setFillColor(svc.styles.colors.ui_white);
	title.setLetterSpacing(2.f);

	widget_font.loadFromFile(svc.text.title_font);
	widget_font.setSmooth(false);
	widget_label.setCharacterSize(ui.widget_size);
	widget_label.setFont(widget_font);
	widget_label.setFillColor(svc.styles.colors.ui_white);

	origin = {ui.corner_pad * 0.5f, ui.corner_pad * 0.5f};
	title.setPosition(origin + ui.title_offset);
	widget_label.setPosition(origin + ui.widget_label_offset);

	final_dimensions = sf::Vector2<float>{svc.constants.screen_dimensions.x - ui.corner_pad, svc.constants.screen_dimensions.y - ui.corner_pad};
	current_dimensions = final_dimensions;
	position = sf::Vector2<float>{origin.x, origin.y};
	speed = 8;
	flags.reset(ConsoleFlags::portrait_included);
	extent = final_dimensions.y;

}

void PauseWindow::update(automa::ServiceProvider& svc, player::Player& player) {
	if (!active()) { return; }
	Console::update(svc);

	selector.update();
}

void PauseWindow::render(automa::ServiceProvider& svc, player::Player& player, sf::RenderWindow& win) {
	if (!active()) { return; }
	Console::render(win);
	win.draw(title);
}

void PauseWindow::open() { flags.set(ConsoleFlags::active); }

void PauseWindow::close() { Console::end(); }

} // namespace gui
