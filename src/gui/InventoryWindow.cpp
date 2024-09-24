#include "InventoryWindow.hpp"
#include "../entities/player/Player.hpp"
#include "../level/Map.hpp"
#include "../service/ServiceProvider.hpp"

namespace gui {

InventoryWindow::InventoryWindow(automa::ServiceProvider& svc) : Console::Console(svc), info(svc), selector(svc, {2, 1}), minimap(svc), item_menu(svc, {"use", "cancel"}, true) {
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

	dimensions = sf::Vector2<float>{svc.constants.screen_dimensions.x - ui.corner_pad, svc.constants.screen_dimensions.y - ui.corner_pad};
	position = svc.constants.f_center_screen;
	flags.reset(ConsoleFlags::portrait_included);
	Console::update(svc);
	sprite.set_position(position);

	info.dimensions = {dimensions.x - 2.f * ui.info_offset.x, dimensions.y * 0.62f - ui.info_offset.y - ui.inner_corner};
	info.position = svc.constants.f_center_screen;
	info.position.y += ui.info_offset.y;
	info.sprite.set_position(info.position);
	info.flags.reset(ConsoleFlags::portrait_included);
	info.update(svc);

	mode = Mode::inventory;

	help_marker.init(svc, "Press [", config::DigitalAction::platformer_open_map, "] to view Map.", 20, true, true); // XXX this was arms_switch_right
	help_marker.set_position({static_cast<float>(svc.constants.screen_dimensions.x) * 0.5f, static_cast<float>(svc.constants.screen_dimensions.y) - 30.f});
}

void InventoryWindow::update(automa::ServiceProvider& svc, player::Player& player, world::Map& map) {
	if (mode == Mode::inventory) {
		title.setString("INVENTORY");
		auto& player_items = player.catalog.categories.inventory.items;
		if (active()) {
			Console::update(svc);
			if (Console::extended()) { info.active() ? info.update(svc) : info.begin(); }
		} else {
			info.update(svc);
		}
		auto x_dim = std::min(static_cast<int>(player_items.size()), ui.items_per_row);
		auto y_dim = static_cast<int>(std::ceil(static_cast<float>(player_items.size()) / static_cast<float>(ui.items_per_row)));
		selector.update();
		for (auto& item : player_items) {
			item.selection_index == selector.get_current_selection() ? item.select() : item.deselect();
			if (player_items.size() == 1) { item.select(); }
			if (item.selected() && info.extended()) {
				selector.set_position(item.get_position());
				info.writer.load_single_message(item.get_description());
				info.writer.wrap();
				item.set_rarity_position(info.position + info.dimensions * 0.5f - ui.rarity_pad);
			}
		}
		info.update(svc);
		if (active()) {
			if (svc.controller_map.digital_action_status(config::DigitalAction::menu_select).triggered) {
				if (item_menu.is_open()) {
					switch (item_menu.get_selection()) {
					case 0: 
						if (selector.get_current_selection() < player_items.size()) { use_item(svc, player, map, player_items.at(selector.get_current_selection())); }
						break;
					case 1:
						item_menu.close(svc);
						svc.soundboard.flags.menu.set(audio::Menu::backward_switch);
						break;
					}
				} else {
					if (selector.get_current_selection() < player_items.size()) {
						if (player_items.at(selector.get_current_selection()).usable()) {
							item_menu.open(svc, selector.get_menu_position());
							svc.soundboard.flags.console.set(audio::Console::menu_open);
						} else {
							svc.soundboard.flags.menu.set(audio::Menu::backward_switch);
						}
					}
				}
			}
			if (svc.controller_map.digital_action_status(config::DigitalAction::menu_cancel).triggered) {
				if (item_menu.is_open()) {
					item_menu.close(svc);
					svc.soundboard.flags.menu.set(audio::Menu::backward_switch);
				}
			}
		}
		auto minimenu_dim = sf::Vector2<float>{108.f, 108.f};
		item_menu.update(svc, minimenu_dim, selector.get_menu_position());
	}
	if (mode == Mode::minimap) {
		title.setString("MAP");
		Console::update(svc);
		minimap.update(svc, map, player);
		selector.update();
	}
}

void InventoryWindow::render(automa::ServiceProvider& svc, player::Player& player, sf::RenderWindow& win, sf::Vector2<float> cam) {
	if (!active()) { return; }
	if (mode == Mode::inventory) {
	Console::render(win);
	if (!Console::extended()) { return; }
	win.draw(title);
		for (auto& item : player.catalog.categories.inventory.items) {
			item.render(svc, win, {0.f, 0.f});
			if (item.selected()) {
				item_label.setString(item.get_label().data());
				win.draw(item_label);
			}
		}
		if (!player.catalog.categories.inventory.items.empty() && info.extended()) { selector.render(win); }
		if (info.active()) { info.render(win); }
		if (info.extended()) { info.write(win, true); }
		if (player.has_map()) { help_marker.render(win); }
		item_menu.render(win);
	}
	if (mode == Mode::minimap) {
		minimap.render(svc, win, cam);
	}
}

void InventoryWindow::open() {
	flags.set(ConsoleFlags::active);
	Console::begin();
	info.begin();
}

void InventoryWindow::close() {
	Console::end();
	info.end();
}

void InventoryWindow::select() {
	//item_menu.open(*m_services, selector.get_menu_position());
	//m_services->soundboard.flags.console.set(audio::Console::menu_open);
}

void InventoryWindow::cancel() { //item_menu.close(*m_services); 
}

void InventoryWindow::move(sf::Vector2<int> direction) {
	if (item_menu.is_open()) {
		if (direction.y == -1) { item_menu.up(*m_services); }
		if (direction.y == 1) { item_menu.down(*m_services); }
	} else {
		if (direction.x == -1) { selector.go_left(); }
		if (direction.x == 1) { selector.go_right(); }
		if (direction.y == -1) { selector.go_up(); }
		if (direction.y == 1) { selector.go_down(); }
	}
}

void InventoryWindow::use_item(automa::ServiceProvider& svc, player::Player& player, world::Map& map, item::Item& item) {
	if (!item.usable()) { return; }
	switch (item.get_id()) {
	case 16:
		mode = Mode::minimap;
		svc.soundboard.flags.menu.set(audio::Menu::select);
		break;
	case 22:
		player.health.refill();
		svc.soundboard.flags.world.set(audio::World::soft_sparkle);
		svc.soundboard.flags.item.set(audio::Item::heal);
		player.take_item(22);
		break;
	}
	item_menu.close(svc);
}

void InventoryWindow::switch_modes(automa::ServiceProvider& svc) {
	mode = (mode == Mode::inventory) ? Mode::minimap : Mode::inventory;
	if (mode == Mode::inventory) {
		help_marker.init(svc, "Press [", config::DigitalAction::platformer_open_map, "] to view Map.", 20, true, true); // XXX same as above
	} else {
		help_marker.init(svc, "Press [", config::DigitalAction::platformer_open_inventory, "] to view Inventory.", 20, true, true); // XXX same as above
		minimap.center();
	}
	help_marker.set_position({static_cast<float>(svc.constants.screen_dimensions.x) * 0.5f, static_cast<float>(svc.constants.screen_dimensions.y) - 30.f});
	svc.soundboard.flags.menu.set(audio::Menu::forward_switch);
}

} // namespace gui
