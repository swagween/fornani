#include "InventoryWindow.hpp"
#include "../service/ServiceProvider.hpp"
#include "../entities/player/Player.hpp"
#include "../level/Map.hpp"

namespace gui {

InventoryWindow::InventoryWindow(automa::ServiceProvider& svc) : Console::Console(svc), info(svc), selector(svc, {2, 1}), minimap(svc) {
	title.setString("INVENTORY");
	title.setCharacterSize(ui.title_size);
	title_font.loadFromFile(svc.text.title_font);
	title_font.setSmooth(false);
	title.setFont(title_font);
	title.setFillColor(svc.styles.colors.ui_white);
	title.setLetterSpacing(2.f);

	item_font.loadFromFile(svc.text.title_font);
	item_font.setSmooth(false);
	item_label.setCharacterSize(ui.desc_size);
	item_label.setFont(item_font);
	item_label.setFillColor(svc.styles.colors.ui_white);

	info.set_texture(svc.assets.t_console_outline);

	origin = {ui.corner_pad * 0.5f, ui.corner_pad * 0.5f};
	title.setPosition(origin + ui.title_offset);
	item_label.setPosition(origin + ui.item_label_offset);

	final_dimensions = sf::Vector2<float>{svc.constants.screen_dimensions.x - ui.corner_pad, svc.constants.screen_dimensions.y - ui.corner_pad};
	current_dimensions = final_dimensions;
	position = sf::Vector2<float>{origin.x, origin.y};
	speed = 8;
	flags.reset(ConsoleFlags::portrait_included);

	info.final_dimensions = {final_dimensions.x - 2.f * ui.info_offset.x, final_dimensions.y - ui.info_offset.y - ui.inner_corner};
	info.current_dimensions = info.final_dimensions;
	info.position = sf::Vector2<float>{origin + ui.info_offset};
	info.speed = 8;
	info.flags.reset(ConsoleFlags::portrait_included);

	mode = Mode::inventory;

	help_marker.init(svc, "Press [", "arms_switch_right", "] to view Map.", 20, true, true);
	help_marker.set_position({static_cast<float>(svc.constants.screen_dimensions.x) * 0.5f, static_cast<float>(svc.constants.screen_dimensions.y) - 30.f});

}

void InventoryWindow::update(automa::ServiceProvider& svc, player::Player& player, world::Map& map) {
	if (mode == Mode::inventory) {
		title.setString("INVENTORY");
		if (active()) {
			extent = final_dimensions.y;
			info.extent = info.final_dimensions.y;
			Console::update(svc);
			if (Console::extended()) { info.update(svc); }
		}
		for (auto& item : player.catalog.categories.inventory.items) {
			item.selection_index == selector.get_current_selection() ? item.select() : item.deselect();
			if (player.catalog.categories.inventory.items.size() == 1) { item.select(); }
			if (item.selected()) {
				selector.set_position(item.get_position());
				info.writer.load_single_message(item.get_description());
			}
		}
		auto x_dim = std::min(static_cast<int>(player.catalog.categories.inventory.items.size()), ui.items_per_row);
		auto y_dim = static_cast<int>(std::ceil(static_cast<float>(player.catalog.categories.inventory.items.size()) / static_cast<float>(ui.items_per_row)));

		selector.set_dimensions({x_dim, y_dim});
		selector.update();
	}
	if (mode == Mode::minimap) {
		title.setString("MAP");
		minimap.update(svc, map, player);
		selector.update();
	}
}

void InventoryWindow::render(automa::ServiceProvider& svc, player::Player& player, sf::RenderWindow& win, sf::Vector2<float> cam) {
	if (!active()) { return; }
	Console::render(win);
	win.draw(title);
	if (mode == Mode::inventory) {
		for (auto& item : player.catalog.categories.inventory.items) {
			item.render(svc, win, {0.f, 0.f});
			if (item.selected()) {
				item_label.setString(item.get_label().data());
				if (Console::extended()) { win.draw(item_label); }
			}
		}
		if (!player.catalog.categories.inventory.items.empty()) { selector.render(win); }
		if (Console::extended()) {
			info.begin();
			info.render(win);
			if (info.extended()) { info.write(win, true); }
		}
		if (player.has_map()) { help_marker.render(win); }
	}
	if (mode == Mode::minimap) {
		help_marker.render(win);
		minimap.render(svc, win, cam);
	}
}

void InventoryWindow::open() { flags.set(ConsoleFlags::active); }

void InventoryWindow::close() {
	Console::end();
	info.end();
}

void InventoryWindow::switch_modes(automa::ServiceProvider& svc) {
	mode = (mode == Mode::inventory) ? Mode::minimap : Mode::inventory;
	if (mode == Mode::inventory) {
		help_marker.init(svc, "Press [", "arms_switch_right", "] to view Map.", 20, true, true);
	} else {
		help_marker.init(svc, "Press [", "arms_switch_left", "] to view Inventory.", 20, true, true);
		minimap.center();
	}
	help_marker.set_position({static_cast<float>(svc.constants.screen_dimensions.x) * 0.5f, static_cast<float>(svc.constants.screen_dimensions.y) - 30.f});
}


} // namespace gui
