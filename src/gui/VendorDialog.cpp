#include "VendorDialog.hpp"
#include <algorithm>
#include "../service/ServiceProvider.hpp"
#include "../level/Map.hpp"
#include "../entities/player/Player.hpp"

namespace gui {

VendorDialog::VendorDialog(automa::ServiceProvider& svc, world::Map& map, player::Player& player, int vendor_id)
	: vendor_id(vendor_id), portrait(svc), info(svc), selector(svc, {2, 1}), item_menu(svc, {"sell", "cancel"}, true) {
	artwork.setTexture(svc.assets.t_vendor_artwork);
	ui.setTexture(svc.assets.t_vendor_ui);
	flags.set(VendorDialogStatus::opened);
	artwork.setTextureRect(sf::IntRect{{0, (vendor_id - 1) * svc.constants.screen_dimensions.y}, {svc.constants.screen_dimensions}});
	state = VendorState::buy;
	ui.setTextureRect(sf::IntRect{{0, static_cast<int>(state) * svc.constants.screen_dimensions.y}, {svc.constants.screen_dimensions}});
	get_npc_id.insert({1, 3});
	npc_id = get_npc_id.at(vendor_id);
	portrait.set_id(npc_id);
	portrait.set_position(portrait_position);
	info.set_texture(svc.assets.t_console_outline);

	info.dimensions = {svc.constants.f_screen_dimensions.x - 48.f, 104.f};
	info.position = {svc.constants.f_center_screen.x, 448.f};
	info.sprite.set_position(info.position);
	info.flags.reset(ConsoleFlags::portrait_included);
	info.begin();
	info.update(svc);

	// text
	text.vendor_name.setString(svc.tables.get_npc_label_formatted(npc_id));
	text.orb_count.setString("ORBS: " + player.wallet.get_balance_string());
	text.buy_tab.setString("BUY");
	text.sell_tab.setString("SELL");
	text.price.setString("price: ");
	// font
	text.vendor_name.setFont(svc.text.fonts.title);
	text.orb_count.setFont(svc.text.fonts.title);
	text.buy_tab.setFont(svc.text.fonts.title);
	text.sell_tab.setFont(svc.text.fonts.title);
	text.price.setFont(svc.text.fonts.title);
	text.item_label.setFont(svc.text.fonts.title);
	text.vendor_name.setCharacterSize(16);
	text.orb_count.setCharacterSize(16);
	text.buy_tab.setCharacterSize(16);
	text.sell_tab.setCharacterSize(16);
	text.price.setCharacterSize(16);
	text.item_label.setCharacterSize(16);
	// colors
	text.vendor_name.setFillColor(svc.styles.colors.ui_white);
	text.orb_count.setFillColor(svc.styles.colors.ui_white);
	text.buy_tab.setFillColor(svc.styles.colors.ui_white);
	text.sell_tab.setFillColor(svc.styles.colors.ui_white);
	text.price.setFillColor(svc.styles.colors.ui_white);
	text.item_label.setFillColor(svc.styles.colors.ui_white);
	// origins
	text.vendor_name.setOrigin(text.vendor_name.getLocalBounds().getSize() * 0.5f);
	text.buy_tab.setOrigin({text.buy_tab.getLocalBounds().getSize().x * 0.5f, 0.f});
	text.sell_tab.setOrigin({text.sell_tab.getLocalBounds().getSize().x * 0.5f, 0.f});
	// positions
	text.vendor_name.setPosition({108, 290});
	text.buy_tab.setPosition({276, 28});
	text.sell_tab.setPosition({404, 28});
	text.price.setPosition(ui_constants.price_position);
	text.item_label.setPosition(ui_constants.item_label_position);

	auto& sellable_items = player.catalog.categories.inventory.sellable_items;
	auto& player_inventory = player.catalog.categories.inventory;
	for (auto& idx : sellable_items) {
		auto& item = player_inventory.get_item_at_index(idx);
		auto randx = svc.random.random_range_float(0.f, svc.constants.f_screen_dimensions.x);
		auto randy = svc.random.random_range_float(0.f, svc.constants.f_screen_dimensions.y);
		auto top = svc.random.percent_chance(50);
		auto startpos = top ? sf::Vector2<float>{randx, -ui_constants.buffer} : sf::Vector2<float>{-ui_constants.buffer, randy};
		item.gravitator.set_position(startpos);
		item.set_offset({214.f - 30.f, 62.f - 20.f});
	}
	update_table(player, true);
}

void VendorDialog::update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {
	auto& sellable_items = player.catalog.categories.inventory.sellable_items;
	auto& player_inventory = player.catalog.categories.inventory;
	auto& vendor = map.get_npc(npc_id).get_vendor();
	player.catalog.update(svc);
	if (svc.controller_map.digital_action_status(config::DigitalAction::menu_left).triggered) { selector.go_left(); }
	if (svc.controller_map.digital_action_status(config::DigitalAction::menu_right).triggered) { selector.go_right(); }
	if (svc.controller_map.digital_action_status(config::DigitalAction::menu_up).triggered) { selector.go_up(); }
	if (svc.controller_map.digital_action_status(config::DigitalAction::menu_down).triggered) { selector.go_down(); }
	if (svc.controller_map.digital_action_status(config::DigitalAction::menu_cancel).triggered) { close(); }
	if (svc.controller_map.digital_action_status(config::DigitalAction::menu_switch_left).triggered || svc.controller_map.digital_action_status(config::DigitalAction::menu_switch_right).triggered) {
		state = state == VendorState::buy ? VendorState::sell : VendorState::buy;
		ui.setTextureRect(sf::IntRect{{0, static_cast<int>(state) * svc.constants.screen_dimensions.y}, {svc.constants.screen_dimensions}});
		svc.soundboard.flags.menu.set(audio::Menu::forward_switch);
	}
	for (auto& idx : sellable_items) {
		auto& item = player_inventory.get_item_at_index(idx);
		if (!flags.test(VendorDialogStatus::opened)) { item.set_offset({}); }
	}
	switch (state) {
	case VendorState::buy: break;
	case VendorState::sell:
		item_menu.overwrite_option(0, "sell");
		selector.set_size(sellable_items.size());
		auto ctr{0};
		for (auto& idx : sellable_items) {
			auto& item = player_inventory.get_item_at_index(idx);
			item.update(svc, ctr, ui_constants.items_per_row);
			item.selection_index == selector.get_current_selection() ? item.select() : item.deselect();
			if (selector.get_section() != InventorySection::item) { item.deselect(); }
			if (selector.get_section() == InventorySection::item) {
				if (sellable_items.size() == 1) { item.select(); }
				if (item.depleted()) { selector.go_left(); }
				if (item.selected() && info.extended()) {
					selector.set_position(item.get_position());
					info.writer.load_single_message(item.get_description());
					info.writer.wrap();
					item.set_rarity_position(info.position + info.dimensions * 0.5f - ui_constants.rarity_pad);
					sale_price = item.get_value() + item.get_value() * vendor.get_upcharge();
					text.price.setString("price: " + std::to_string(sale_price));
				}
			}
			++ctr;
		}
		if (svc.controller_map.digital_action_status(config::DigitalAction::menu_select).triggered && selector.get_section() == InventorySection::item) {
			if (item_menu.is_open()) {
				switch (item_menu.get_selection()) {
				case 0:
					if (selector.get_current_selection() < sellable_items.size()) {
						/* sell item */
					}
					break;
				case 1:
					item_menu.close(svc);
					svc.soundboard.flags.menu.set(audio::Menu::backward_switch);
					break;
				}
			} else {
				if (selector.get_current_selection() < sellable_items.size()) {
					auto& item = sellable_items.at(selector.get_current_selection());
					item_menu.open(svc, selector.get_menu_position());
					svc.soundboard.flags.console.set(audio::Console::menu_open);
				} else {
					svc.soundboard.flags.menu.set(audio::Menu::backward_switch);
				}
			}
		}
		break;
	}
	info.update(svc);
	auto minimenu_dim = sf::Vector2<float>{108.f, 108.f};
	item_menu.update(svc, minimenu_dim, selector.get_menu_position());
	update_table(player, selector.switched_sections());
}

void VendorDialog::render(automa::ServiceProvider& svc, sf::RenderWindow& win, player::Player& player) {
	auto& sellable_items = player.catalog.categories.inventory.sellable_items;
	auto& player_inventory = player.catalog.categories.inventory;
	text.orb_count.setOrigin({text.orb_count.getLocalBounds().getSize().x, 0.f});
	text.orb_count.setPosition({svc.constants.f_screen_dimensions.x - 72.f, 36.f});
	win.draw(artwork);
	win.draw(ui);
	win.draw(text.vendor_name);
	win.draw(text.buy_tab);
	win.draw(text.sell_tab);
	win.draw(text.orb_count);
	win.draw(text.price);
	portrait.render(win);
	if (!player.catalog.categories.inventory.items.empty() && info.extended() && selector.get_section() == InventorySection::item) { selector.render(win); }
	if (info.active()) { info.render(win); }
	if (info.extended()) { info.write(win, true); }
	switch (state) {
	case VendorState::buy: break;
	case VendorState::sell:
		for (auto& idx : sellable_items) {
			auto& item = player_inventory.get_item_at_index(idx);
			item.render(svc, win, {0.f, 0.f});
			if (item.selected()) {
				text.item_label.setString(item.get_label().data());
				win.draw(text.item_label);
			}
		}
		break;
	}
	selector.render(win);
}

void VendorDialog::close() { flags.reset(VendorDialogStatus::opened); }

void VendorDialog::update_table(player::Player& player, bool new_dim) {
	auto& sellable_items = player.catalog.categories.inventory.sellable_items;
	auto& player_inventory = player.catalog.categories.inventory;
	auto ipr = ui_constants.items_per_row;
	auto x_dim{0};
	auto y_dim{0};
	x_dim = std::min(static_cast<int>(sellable_items.size()), ipr);
	y_dim = static_cast<int>(std::ceil(static_cast<float>(sellable_items.size()) / static_cast<float>(ipr)));
	if (new_dim) { selector.set_size(static_cast<int>(sellable_items.size())); }
	selector.set_dimensions({x_dim, y_dim});
	selector.update();
}

} // namespace gui