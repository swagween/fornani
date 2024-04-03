#include "InventoryWindow.hpp"
#include "../service/ServiceProvider.hpp"
#include "../entities/player/Player.hpp"

namespace gui {

InventoryWindow::InventoryWindow(automa::ServiceProvider& svc) : Console::Console(svc) {
	title.setString("INVENTORY");
	title.setCharacterSize(ui.title_size);
	title_font.loadFromFile(svc.text.font);
	title_font.setSmooth(false);
	title.setFont(title_font);
	title.setColor(svc.styles.colors.ui_white);
	title.setLetterSpacing(2.f);

	origin = {ui.corner_pad * 0.5f, ui.corner_pad * 0.5f};
	title.setPosition(origin + ui.title_offset);
	final_dimensions = sf::Vector2<float>{svc.constants.screen_dimensions.x - ui.corner_pad, svc.constants.screen_dimensions.y - ui.corner_pad};
	current_dimensions = final_dimensions;
	position = sf::Vector2<float>{origin.x, origin.y};
	speed = 8;
	flags.reset(ConsoleFlags::portrait_included);
}

void InventoryWindow::update(automa::ServiceProvider& svc, player::Player& player) {
	if (active()) { Console::update(svc); }
	for (auto& item : player.catalog.categories.inventory.items) { item.selection_index == selector.get_current_selection() ? item.select() : item.deselect(); }
	auto x_dim = std::min((int)player.catalog.categories.inventory.items.size(), ui.items_per_row);
	auto y_dim = (int)std::ceil((float)player.catalog.categories.inventory.items.size() / (float)ui.items_per_row);

	selector.set_dimensions({x_dim, y_dim});
}



void InventoryWindow::render(automa::ServiceProvider& svc, player::Player& player, sf::RenderWindow& win) {
	if (active()) {
		Console::render(win);
		win.draw(title);
		for (auto& item : player.catalog.categories.inventory.items) { item.render(svc, win, {0.f, 0.f}); }
	}
}

void InventoryWindow::open() { flags.set(ConsoleFlags::active); }

void InventoryWindow::close() { Console::end(); }

} // namespace gui
