#include "VendorDialog.hpp"
#include <algorithm>
#include "../service/ServiceProvider.hpp"
#include "../level/Map.hpp"
#include "../entities/player/Player.hpp"
#include "../graphics/Transition.hpp"
#include "../../include/fornani/utils/Math.hpp"

namespace gui {

VendorDialog::VendorDialog(automa::ServiceProvider& svc, world::Map& map, player::Player& player, int vendor_id)
	: vendor_id(vendor_id), portrait(svc), info(svc), selectors{.buy{svc, {2, 1}}, .sell{svc, {2, 1}}}, item_menu(svc, {"sell", "cancel"}, true), orb{.sprite{svc.assets.t_orb, {24, 24}}}, artwork{svc.assets.t_vendor_artwork},
	  ui{svc.assets.t_vendor_ui}, text{.vendor_name{svc.text.fonts.title},
									   .buy_tab{svc.text.fonts.title},
									   .sell_tab{svc.text.fonts.title},
									   .orb_count{svc.text.fonts.title},
									   .price{svc.text.fonts.title},
									   .price_number{svc.text.fonts.title},
									   .item_label{svc.text.fonts.title}} {
	flags.set(VendorDialogStatus::opened);
	artwork.setTextureRect(sf::IntRect{{0, (vendor_id - 1) * svc.constants.screen_dimensions.y}, {svc.constants.screen_dimensions}});
	artwork.setOrigin(svc.constants.f_center_screen);
	artwork.setPosition(svc.constants.f_center_screen);
	state = VendorState::buy;
	ui.setTextureRect(sf::IntRect{{0, static_cast<int>(state) * svc.constants.screen_dimensions.y}, {svc.constants.screen_dimensions}});
	get_npc_id.insert({1, 3});
	npc_id = get_npc_id.at(vendor_id);
	portrait.set_id(npc_id);
	info.set_texture(svc.assets.t_console_outline);

	info.dimensions = {svc.constants.f_screen_dimensions.x - 48.f, 110.f};
	info.position = {svc.constants.f_center_screen.x, 450.f};
	info.sprite.set_position(info.position);
	info.flags.reset(ConsoleFlags::portrait_included);
	info.begin();
	info.update(svc);

	auto& in_anim = svc.data.drop["orb"]["animation"];
	for (auto& param : in_anim["params"].array_view()) {
		anim::Parameters a{};
		a.duration = param["duration"].as<int>();
		a.framerate = param["framerate"].as<int>();
		a.num_loops = param["num_loops"].as<int>();
		orb.sprite.push_params(param["label"].as_string(), a);
	}
	orb.sprite.set_params("neutral", true);

	// text
	text.vendor_name.setString(svc.tables.get_npc_label_formatted(npc_id));
	text.orb_count.setString(player.wallet.get_balance_string());
	text.buy_tab.setString("BUY");
	text.sell_tab.setString("SELL");
	text.price.setString("PRICE: ");
	// font
	text.vendor_name.setCharacterSize(16);
	text.orb_count.setCharacterSize(16);
	text.buy_tab.setCharacterSize(16);
	text.sell_tab.setCharacterSize(16);
	text.price.setCharacterSize(16);
	text.item_label.setCharacterSize(16);
	text.price_number.setCharacterSize(16);
	// colors
	text.vendor_name.setFillColor(svc.styles.colors.ui_white);
	text.orb_count.setFillColor(svc.styles.colors.ui_white);
	text.price.setFillColor(svc.styles.colors.red);
	text.item_label.setFillColor(svc.styles.colors.red);
	text.price_number.setFillColor(svc.styles.colors.periwinkle);
	// origins
	text.vendor_name.setOrigin(text.vendor_name.getLocalBounds().getCenter());
	text.buy_tab.setOrigin({text.buy_tab.getLocalBounds().getCenter().x, 0.f});
	text.sell_tab.setOrigin({text.sell_tab.getLocalBounds().getCenter().x, 0.f});

	refresh(svc, player, map);
	init = true;
	intro.start();
}

void VendorDialog::update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {
	if (init) {
		update_table(player, map, true);
		init = false;
	}
	intro.update();
	if (intro.is_almost_complete()) {
		bring_in = {svc.constants.f_screen_dimensions.x, 0.f};
		bring_in_cooldown.start();
	}
	if (intro.running()) {
		auto scale = util::ease_in_out(1.f, 1.3f, intro.get_normalized());
		artwork.setScale({scale, scale});
		return;
	}
	bring_in_cooldown.update();
	if (bring_in_cooldown.running()) {
		auto dest = svc.constants.f_screen_dimensions.x;
		bring_in.x = util::ease_in_out(dest, 0.f, 1.f - bring_in_cooldown.get_normalized());
	} else {
		bring_in = {};
	}

	artwork.setScale({1.f, 1.f});

	// asset positions
	text.vendor_name.setPosition(sf::Vector2<float>{108, 290} + bring_in);
	text.buy_tab.setPosition(sf::Vector2<float>{276, 28} + bring_in);
	text.sell_tab.setPosition(sf::Vector2<float>{404, 28} + bring_in);
	text.price.setPosition(ui_constants.price_position + bring_in);
	text.item_label.setPosition(ui_constants.item_label_position + bring_in);
	text.price_number.setPosition(sf::Vector2<float>{text.price.getPosition().x + text.price.getLocalBounds().size.x, text.price.getPosition().y} + bring_in);
	ui.setPosition(bring_in);
	portrait.set_position(portrait_position + bring_in);
	text.orb_count.setPosition(sf::Vector2<float>{svc.constants.f_screen_dimensions.x - 72.f, 36.f} + bring_in);

	state == VendorState::buy ? text.buy_tab.setFillColor(svc.styles.colors.red) : text.buy_tab.setFillColor(svc.styles.colors.blue);
	state == VendorState::sell ? text.sell_tab.setFillColor(svc.styles.colors.red) : text.sell_tab.setFillColor(svc.styles.colors.blue);
	bool exchanged{};
	orb.sprite.update(sf::Vector2<float>{svc.constants.f_screen_dimensions.x - 60.f, 32.f} + bring_in, 0, 0, true);
	text.orb_count.setString(player.wallet.get_balance_string());
	if (opening()) { return; }

	auto& sellable_items = player.catalog.categories.inventory.sellable_items;
	auto& player_inventory = player.catalog.categories.inventory;
	auto& vendor = map.get_npc(npc_id).get_vendor();
	auto& selector = state == VendorState::sell ? selectors.sell : selectors.buy;

	if (!item_menu.is_open()) {
		if (svc.controller_map.digital_action_status(config::DigitalAction::menu_left).triggered) { selector.go_left(); }
		if (svc.controller_map.digital_action_status(config::DigitalAction::menu_right).triggered) { selector.go_right(); }
		if (svc.controller_map.digital_action_status(config::DigitalAction::menu_up).triggered) { selector.go_up(); }
		if (svc.controller_map.digital_action_status(config::DigitalAction::menu_down).triggered) { selector.go_down(); }
		if (svc.controller_map.digital_action_status(config::DigitalAction::menu_cancel).triggered) { close(); }
	} else {
		if (svc.controller_map.digital_action_status(config::DigitalAction::menu_up).triggered) { item_menu.down(svc); }
		if (svc.controller_map.digital_action_status(config::DigitalAction::menu_down).triggered) { item_menu.up(svc); }
		if (svc.controller_map.digital_action_status(config::DigitalAction::menu_cancel).triggered) { item_menu.close(svc); }
	}
	if (!item_menu.is_open() && (svc.controller_map.digital_action_status(config::DigitalAction::menu_switch_left).triggered || svc.controller_map.digital_action_status(config::DigitalAction::menu_switch_right).triggered)) {
		state = state == VendorState::buy ? VendorState::sell : VendorState::buy;
		selector.switch_sections({1, 0});
		update_table(player, map, true);
		ui.setTextureRect(sf::IntRect{{0, static_cast<int>(state) * svc.constants.screen_dimensions.y}, {svc.constants.screen_dimensions}});
		svc.soundboard.flags.menu.set(audio::Menu::forward_switch);
		refresh(svc, player, map);
	}

	for (auto& idx : sellable_items) {
		auto& item = player_inventory.get_item_at_index(idx);
		if (!flags.test(VendorDialogStatus::opened)) { item.set_offset({}); }
	}
	auto ctr{0};
	switch (state) {
	case VendorState::buy:
		if (vendor.inventory.items.empty()) { break; }
		ctr = 0;
		item_menu.overwrite_option(0, "buy");
		selector.update();
		selector.set_size(static_cast<int>(vendor.inventory.items.size()));
		if (vendor.inventory.items.size() < 1) { text.price_number.setString("..."); }
		for (auto& item : vendor.inventory.items) {
			item.update(svc, ctr, ui_constants.items_per_row);
			ctr == selector.get_current_selection() ? item.select() : item.deselect();
			if (item.selected()) {
				selector.set_position(item.get_position());
				if (info.extended()) {
					info.writer.load_single_message(item.get_description());
					info.writer.wrap();
				}
				item.set_rarity_position(info.position + info.dimensions * 0.5f - ui_constants.rarity_pad);
				auto f_value = static_cast<float>(item.get_value());
				sale_price = f_value + f_value * vendor.get_upcharge();
				text.price_number.setString(std::format("{}", sale_price));
			}
			++ctr;
		}
		if (svc.controller_map.digital_action_status(config::DigitalAction::menu_select).triggered) {
			if (item_menu.is_open()) {
				switch (item_menu.get_selection()) {
				case 0:
					if (selector.get_current_selection() < vendor.inventory.items.size()) {
						if (player.wallet.get_balance() <= sale_price) {
							svc.soundboard.flags.menu.set(audio::Menu::backward_switch);
							item_menu.close(svc);
							break;
						}
						auto item_id = vendor.inventory.items.at(selector.get_current_selection()).get_id();
						player.give_item(item_id, 1);
						player.give_drop(item::DropType::orb, -sale_price);
						balance -= sale_price;
						vendor.inventory.remove_item(svc, item_id, 1);
						svc.soundboard.flags.world.set(audio::World::soft_sparkle_high);
						exchanged = true;
					}
					item_menu.close(svc);
					break;
				case 1:
					item_menu.close(svc);
					svc.soundboard.flags.menu.set(audio::Menu::backward_switch);
					break;
				}
			} else {
				if (selector.get_current_selection() < vendor.inventory.items.size()) {
					auto& item = vendor.inventory.items.at(selector.get_current_selection());
					item_menu.open(svc, selector.get_menu_position());
					svc.soundboard.flags.console.set(audio::Console::menu_open);
				} else {
					svc.soundboard.flags.menu.set(audio::Menu::backward_switch);
				}
			}
		}
		break;
	case VendorState::sell:
		if (sellable_items.empty()) { break; }
		ctr = 0;
		item_menu.overwrite_option(0, "sell");
		selector.set_size(static_cast<int>(sellable_items.size()));
		selector.update();
		if (sellable_items.size() < 1) { text.price_number.setString("..."); }
		for (auto& idx : sellable_items) {
			auto& item = player_inventory.get_item_at_index(idx);
			item.update(svc, ctr, ui_constants.items_per_row);
			ctr == selector.get_current_selection() ? item.select() : item.deselect();
			if (item.selected()) {
				selector.set_position(item.get_position());
				if (info.extended()) {
					info.writer.load_single_message(item.get_description());
					info.writer.wrap();
				}
				item.set_rarity_position(info.position + info.dimensions * 0.5f - ui_constants.rarity_pad);
				auto f_value = static_cast<float>(item.get_value());
				sale_price = f_value - f_value * vendor.get_upcharge();
				text.price_number.setString(std::format("{}", sale_price));
			}
			++ctr;
		}
		if (svc.controller_map.digital_action_status(config::DigitalAction::menu_select).triggered) {
			if (item_menu.is_open()) {
				switch (item_menu.get_selection()) {
				case 0:
					if (selector.get_current_selection() < sellable_items.size()) {
						auto& item = player_inventory.get_item_at_index(sellable_items.at(selector.get_current_selection()));
						if (item.is_equipped()) { player.unequip_item(item.get_apparel_type(), item.get_id()); }
						vendor.inventory.add_item(svc, item.get_id(), 1);
						player.take_item(item.get_id());
						player.give_drop(item::DropType::orb, sale_price);
						balance += sale_price;
						svc.soundboard.flags.world.set(audio::World::soft_sparkle_high);
						flags.set(VendorDialogStatus::made_sale);
						exchanged = true;
					}
					item_menu.close(svc);
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
	update_table(player, map, true);
}

void VendorDialog::render(automa::ServiceProvider& svc, sf::RenderWindow& win, player::Player& player, world::Map& map) {
	auto& sellable_items = player.catalog.categories.inventory.sellable_items;
	auto& player_inventory = player.catalog.categories.inventory;
	auto& vendor = map.get_npc(npc_id).get_vendor();
	auto& selector = state == VendorState::sell ? selectors.sell : selectors.buy;

	text.orb_count.setOrigin(sf::Vector2<float>{text.orb_count.getLocalBounds().size.x, 0.f});
	win.draw(artwork);
	win.draw(ui);
	win.draw(text.vendor_name);
	win.draw(text.buy_tab);
	win.draw(text.sell_tab);
	win.draw(text.orb_count);
	win.draw(text.price);
	win.draw(text.price_number);
	orb.sprite.render(svc, win, {});
	portrait.render(win);
	if (info.active()) { info.render(win); }
	if (info.extended()) { info.write(win, true); }
	switch (state) {
	case VendorState::buy:
		if (vendor.inventory.items.empty()) { break; }
		if (!opening()) { selector.render(win); }
		for (auto& item : vendor.inventory.items) {
			if (!opening()) { item.render(svc, win, {0.f, 0.f}); }
			if (item.selected()) {
				text.item_label.setString(item.get_label().data());
				win.draw(text.item_label);
			}
		}
		break;
	case VendorState::sell:
		if (player.catalog.categories.inventory.sellable_items.empty()) { break; }
		if (!opening()) { selector.render(win); }
		for (auto& idx : sellable_items) {
			auto& item = player_inventory.get_item_at_index(idx);
			if (!opening()) { item.render(svc, win, {0.f, 0.f}); }
			if (item.selected()) {
				text.item_label.setString(item.get_label().data());
				win.draw(text.item_label);
			}
		}
		break;
	}
	item_menu.render(win);
	if (intro.running()) { win.draw(artwork); }
}

void VendorDialog::close() { flags.reset(VendorDialogStatus::opened); }

void VendorDialog::update_table(player::Player& player, world::Map& map, bool new_dim) {
	auto& vendor = map.get_npc(npc_id).get_vendor();
	auto& selector = state == VendorState::sell ? selectors.sell : selectors.buy;
	auto num_items = (state == VendorState::sell) ? player.catalog.categories.inventory.sellable_items.size() : vendor.inventory.items.size();
	auto ipr = ui_constants.items_per_row;
	auto dim = sf::Vector2<int>{std::min(static_cast<int>(num_items), ipr), static_cast<int>(std::ceil(static_cast<float>(num_items) / static_cast<float>(ipr)))};
	if (new_dim) { selector.set_size(static_cast<int>(num_items)); }
	selector.set_dimensions(dim);
	selector.update();
}

void VendorDialog::refresh(automa::ServiceProvider& svc, player::Player& player, world::Map& map) {
	auto& sellable_items = player.catalog.categories.inventory.sellable_items;
	auto& player_inventory = player.catalog.categories.inventory;
	auto& vendor = map.get_npc(npc_id).get_vendor();
	for (auto& idx : sellable_items) {
		auto& item = player_inventory.get_item_at_index(idx);
		auto randv = svc.random.random_vector_float(-16.f, 16.f);
		auto startpos = item.get_position() + randv;
		item.gravitator.set_position(startpos);
		item.set_offset(sf::Vector2<float>{214.f - 24.f, 62.f - 24.f});
	}
	for (auto& item : vendor.inventory.items) {
		auto randv = svc.random.random_vector_float(-16.f, 16.f);
		auto startpos = item.get_position() + randv;
		item.gravitator.set_position(startpos);
		item.set_offset(sf::Vector2<float>{214.f - 24.f, 62.f - 24.f});
	}
}

} // namespace gui