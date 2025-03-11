
#include "fornani/gui/VendorDialog.hpp"
#include "fornani/entities/player/Player.hpp"
#include "fornani/graphics/Transition.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/utils/Math.hpp"
#include "fornani/utils/Random.hpp"
#include "fornani/world/Map.hpp"

namespace fornani::gui {

VendorDialog::VendorDialog(automa::ServiceProvider& svc, world::Map& map, player::Player& player, int vendor_id)
	: vendor_id(vendor_id), portrait(svc), info(svc, "outline_console"), m_buy_selector{{2, 1}}, m_sell_selector{{2, 1}}, orb{.sprite{anim::AnimatedSprite(svc.assets.get_texture("orbs"), {24, 24})}},
	  artwork{svc.assets.get_texture("vendor_background")}, ui{svc.assets.get_texture("vendor_ui")}, text{.vendor_name{svc.text.fonts.title},
																										  .buy_tab{svc.text.fonts.title},
																										  .sell_tab{svc.text.fonts.title},
																										  .orb_count{svc.text.fonts.title},
																										  .price{svc.text.fonts.title},
																										  .price_number{svc.text.fonts.title},
																										  .item_label{svc.text.fonts.title}} {
	flags.set(VendorDialogStatus::opened);
	artwork.setTextureRect(sf::IntRect{{0, (vendor_id - 1) * svc.window->i_screen_dimensions().y}, {svc.window->i_screen_dimensions()}});
	artwork.setOrigin(svc.window->f_center_screen());
	artwork.setPosition(svc.window->f_center_screen());
	state = VendorState::buy;
	ui.setTextureRect(sf::IntRect{{0, static_cast<int>(state) * svc.window->i_screen_dimensions().y}, {svc.window->i_screen_dimensions()}});
	get_npc_id.insert({1, 3});
	npc_id = get_npc_id.at(vendor_id);
	portrait.set_id(npc_id);

	info.flags.reset(ConsoleFlags::portrait_included);
	info.begin();

	for (auto& in_anim = svc.data.drop["orb"]["animation"]; auto& param : in_anim["params"].array_view()) {
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

	refresh(player, map);
	init = true;
	intro.start();
}

// TODO: bring this back
void VendorDialog::update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {
	// if (init) {
	//	update_table(player, map, true);
	//	init = false;
	// }
	// intro.update();
	// if (intro.is_almost_complete()) {
	//	bring_in = {svc.window->f_screen_dimensions().x, 0.f};
	//	bring_in_cooldown.start();
	// }
	// if (intro.running()) {
	//	auto scale = util::ease_in_out(1.f, 1.3f, intro.get_normalized());
	//	artwork.setScale({scale, scale});
	//	return;
	// }
	// bring_in_cooldown.update();
	// if (bring_in_cooldown.running()) {
	//	auto dest = svc.window->f_screen_dimensions().x;
	//	bring_in.x = util::ease_in_out(dest, 0.f, 1.f - bring_in_cooldown.get_normalized());
	// } else {
	//	bring_in = {};
	// }

	// artwork.setScale({1.f, 1.f});

	//// asset positions
	// text.vendor_name.setPosition(sf::Vector2<float>{108, 290} + bring_in);
	// text.buy_tab.setPosition(sf::Vector2<float>{276, 28} + bring_in);
	// text.sell_tab.setPosition(sf::Vector2<float>{404, 28} + bring_in);
	// text.price.setPosition(ui_constants.price_position + bring_in);
	// text.item_label.setPosition(ui_constants.item_label_position + bring_in);
	// text.price_number.setPosition(sf::Vector2<float>{text.price.getPosition().x + text.price.getLocalBounds().size.x, text.price.getPosition().y} + bring_in);
	// ui.setPosition(bring_in);
	// portrait.set_position(portrait_position + bring_in);
	// text.orb_count.setPosition(sf::Vector2<float>{svc.window->f_screen_dimensions().x - 72.f, 36.f} + bring_in);

	// state == VendorState::buy ? text.buy_tab.setFillColor(svc.styles.colors.red) : text.buy_tab.setFillColor(svc.styles.colors.blue);
	// state == VendorState::sell ? text.sell_tab.setFillColor(svc.styles.colors.red) : text.sell_tab.setFillColor(svc.styles.colors.blue);
	// bool exchanged{};
	// orb.sprite.update(sf::Vector2<float>{svc.window->f_screen_dimensions().x - 60.f, 32.f} + bring_in, 0, 0, true);
	// text.orb_count.setString(player.wallet.get_balance_string());
	// if (opening()) { return; }

	// auto& sellable_items = player.catalog.inventory.sellable_items;
	// auto& player_inventory = player.catalog.inventory;
	// auto& vendor = map.get_npc(npc_id).get_vendor();
	// auto& selector = state == VendorState::sell ? m_sell_selector : m_buy_selector;

	// if (!m_item_menu) {
	//	// TODO: refactor input handling in VendorDialog
	//	if (svc.controller_map.digital_action_status(config::DigitalAction::menu_left).triggered) { /*selector.go_left();*/
	//	}
	//	if (svc.controller_map.digital_action_status(config::DigitalAction::menu_right).triggered) { /*selector.go_right();*/
	//	}
	//	if (svc.controller_map.digital_action_status(config::DigitalAction::menu_up).triggered) { /* selector.go_up();*/
	//	}
	//	if (svc.controller_map.digital_action_status(config::DigitalAction::menu_down).triggered) { /*selector.go_down();*/
	//	}
	//	if (svc.controller_map.digital_action_status(config::DigitalAction::menu_cancel).triggered) { close(); }
	// } else {
	//	if (svc.controller_map.digital_action_status(config::DigitalAction::menu_up).triggered) { m_item_menu->down(svc); }
	//	if (svc.controller_map.digital_action_status(config::DigitalAction::menu_down).triggered) { m_item_menu->up(svc); }
	//	if (svc.controller_map.digital_action_status(config::DigitalAction::menu_cancel).triggered) { m_item_menu = {}; }
	// }
	// if (!m_item_menu && (svc.controller_map.digital_action_status(config::DigitalAction::menu_switch_left).triggered || svc.controller_map.digital_action_status(config::DigitalAction::menu_switch_right).triggered)) {
	//	state = state == VendorState::buy ? VendorState::sell : VendorState::buy;
	//	/*selector.switch_sections({1, 0});*/
	//	update_table(player, map, true);
	//	ui.setTextureRect(sf::IntRect{{0, static_cast<int>(state) * svc.window->i_screen_dimensions().y}, {svc.window->i_screen_dimensions()}});
	//	svc.soundboard.flags.menu.set(audio::Menu::forward_switch);
	//	refresh(player, map);
	// }

	// for (auto& idx : sellable_items) {
	//	auto& item = player_inventory.get_item_at_index(idx);
	//	if (!flags.test(VendorDialogStatus::opened)) { item.set_offset({}); }
	// }
	// auto ctr{0};
	// switch (state) {
	// case VendorState::buy:
	//	if (vendor.inventory.items.empty()) { break; }
	//	ctr = 0;
	//	m_item_menu = MiniMenu(svc, {"buy", "cancel"}, selector.get_menu_position(), true);
	//	// TODO: refactor selector dimension updates
	//	/*selector.update();
	//	selector.set_size(static_cast<int>(vendor.inventory.items.size()));*/
	//	if (vendor.inventory.items.size() < 1) { text.price_number.setString("..."); }
	//	for (auto& item : vendor.inventory.items) {
	//		item.update(svc, ctr, ui_constants.items_per_row, {});
	//		ctr == selector.get_current_selection() ? item.select() : item.deselect();
	//		if (item.selected()) {
	//			// TODO: refactor selector dimension updates
	//			/*selector.set_position(item.get_position());*/
	//			info.load_single_message(item.get_description());
	//			auto f_value = static_cast<float>(item.get_value());
	//			sale_price = f_value + f_value * vendor.get_upcharge();
	//			text.price_number.setString(std::format("{}", sale_price));
	//		}
	//		++ctr;
	//	}
	//	if (svc.controller_map.digital_action_status(config::DigitalAction::menu_select).triggered) {
	//		if (m_item_menu) {
	//			switch (m_item_menu->get_selection()) {
	//			case 0:
	//				if (selector.get_current_selection() < vendor.inventory.items.size()) {
	//					if (player.wallet.get_balance() <= sale_price) {
	//						svc.soundboard.flags.menu.set(audio::Menu::backward_switch);
	//						m_item_menu = {};
	//						break;
	//					}
	//					auto item_id = vendor.inventory.items.at(selector.get_current_selection()).get_id();
	//					player.give_item(item_id, 1);
	//					player.give_drop(item::DropType::orb, -sale_price);
	//					balance -= sale_price;
	//					vendor.inventory.remove_item(svc, item_id, 1);
	//					svc.soundboard.flags.item.set(audio::Item::vendor_sale);
	//					exchanged = true;
	//				}
	//				m_item_menu = {};
	//				break;
	//			case 1:
	//				m_item_menu = {};
	//				svc.soundboard.flags.menu.set(audio::Menu::backward_switch);
	//				break;
	//			}
	//		} else {
	//			if (selector.get_current_selection() < vendor.inventory.items.size()) {
	//				m_item_menu = MiniMenu(svc, {"buy", "cancel"}, selector.get_menu_position(), true);
	//				svc.soundboard.flags.console.set(audio::Console::menu_open);
	//			} else {
	//				svc.soundboard.flags.menu.set(audio::Menu::backward_switch);
	//			}
	//		}
	//	}
	//	break;
	// case VendorState::sell:
	//	if (sellable_items.empty()) { break; }
	//	ctr = 0;
	//	m_item_menu = MiniMenu(svc, {"sell", "cancel"}, selector.get_menu_position(), true);
	//	// TODO: refactor selector dimension updates
	//	/*selector.set_size(static_cast<int>(sellable_items.size()));
	//	selector.update();*/
	//	if (sellable_items.size() < 1) { text.price_number.setString("..."); }
	//	for (auto const& idx : sellable_items) {
	//		auto& item = player_inventory.get_item_at_index(idx);
	//		item.update(svc, ctr, ui_constants.items_per_row, {});
	//		ctr == selector.get_current_selection() ? item.select() : item.deselect();
	//		if (item.selected()) {
	//			// TODO: refactor selector dimension updates
	//			/*selector.set_position(item.get_position());*/
	//			info.load_single_message(item.get_description());
	//			auto const f_value = static_cast<float>(item.get_value());
	//			sale_price = f_value - f_value * vendor.get_upcharge();
	//			text.price_number.setString(std::format("{}", sale_price));
	//		}
	//		++ctr;
	//	}
	//	if (svc.controller_map.digital_action_status(config::DigitalAction::menu_select).triggered) {
	//		if (m_item_menu) {
	//			switch (m_item_menu->get_selection()) {
	//			case 0:
	//				if (selector.get_current_selection() < sellable_items.size()) {
	//					auto& item = player_inventory.get_item_at_index(sellable_items.at(selector.get_current_selection()));
	//					if (item.is_equipped()) { player.unequip_item(item.get_apparel_type(), item.get_id()); }
	//					vendor.inventory.add_item(svc, item.get_id(), 1);
	//					player.take_item(item.get_id());
	//					player.give_drop(item::DropType::orb, sale_price);
	//					balance += sale_price;
	//					svc.soundboard.flags.item.set(audio::Item::vendor_sale);
	//					flags.set(VendorDialogStatus::made_sale);
	//					exchanged = true;
	//				}
	//				m_item_menu = {};
	//				break;
	//			case 1:
	//				m_item_menu = {};
	//				svc.soundboard.flags.menu.set(audio::Menu::backward_switch);
	//				break;
	//			}
	//		} else {
	//			if (selector.get_current_selection() < sellable_items.size()) {
	//				m_item_menu = MiniMenu(svc, {"sell", "cancel"}, selector.get_menu_position(), true);
	//				svc.soundboard.flags.console.set(audio::Console::menu_open);
	//			} else {
	//				svc.soundboard.flags.menu.set(audio::Menu::backward_switch);
	//			}
	//		}
	//	}
	//	break;
	// }
	// info.update(svc);
	// constexpr auto minimenu_dim = sf::Vector2{108.f, 108.f};
	// if (m_item_menu) { m_item_menu->update(svc, minimenu_dim, selector.get_menu_position()); }
	// update_table(player, map, true);
}

void VendorDialog::render(automa::ServiceProvider& svc, sf::RenderWindow& win, player::Player& player, world::Map& map) {
	// auto const& sellable_items = player.catalog.inventory.sellable_items;
	auto& player_inventory = player.catalog.inventory;
	auto& vendor = map.get_npc(npc_id).get_vendor();
	auto const& selector = state == VendorState::sell ? m_sell_selector : m_buy_selector;

	text.orb_count.setOrigin(sf::Vector2{text.orb_count.getLocalBounds().size.x, 0.f});
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
	if (info.is_active()) {
		info.render(win);
		info.write(win, true);
	}
	// TODO: update with new Selector rendering
	switch (state) {
	case VendorState::buy:
		// if (vendor.inventory.items.empty()) { break; }
		// if (!opening()) { /*selector.render(win);*/
		// }
		// for (auto& item : vendor.inventory.items) {
		//	if (!opening()) { item.render(svc, win, {0.f, 0.f}); }
		//	if (item.selected()) {
		//		text.item_label.setString(item.get_label().data());
		//		win.draw(text.item_label);
		//	}
		// }
		break;
	case VendorState::sell:
		// if (player.catalog.inventory.sellable_items.empty()) { break; }
		// if (!opening()) { /*selector.render(win);*/
		// }
		// for (auto const& idx : sellable_items) {
		//	auto& item = player_inventory.get_item_at_index(idx);
		//	if (!opening()) { item.render(svc, win, {0.f, 0.f}); }
		//	if (item.selected()) {
		//		text.item_label.setString(item.get_label().data());
		//		win.draw(text.item_label);
		//	}
		// }
		break;
	}
	if (m_item_menu) { m_item_menu->render(win); }
	if (intro.running()) { win.draw(artwork); }
}

void VendorDialog::close() { flags.reset(VendorDialogStatus::opened); }

void VendorDialog::update_table(player::Player& player, world::Map& map, bool new_dim) {
	/*auto const& vendor = map.get_npc(npc_id).get_vendor();
	auto& selector = state == VendorState::sell ? m_sell_selector : m_buy_selector;
	auto const num_items = (state == VendorState::sell) ? player.catalog.inventory.sellable_items.size() : vendor.inventory.items.size();
	auto const ipr = ui_constants.items_per_row;
	auto const dim = sf::Vector2{std::min(static_cast<int>(num_items), ipr), static_cast<int>(std::ceil(static_cast<float>(num_items) / static_cast<float>(ipr)))};*/
	// TODO: refactor selection changes in ui elements
	/*if (new_dim) { selector.set_size(static_cast<int>(num_items)); }
	selector.set_dimensions(dim);
	selector.update();*/
}

void VendorDialog::refresh(player::Player& player, world::Map& map) const {
	/*auto const& sellable_items = player.catalog.inventory.sellable_items;
	auto& player_inventory = player.catalog.inventory;
	auto& vendor = map.get_npc(npc_id).get_vendor();
	for (auto& idx : sellable_items) {
		auto& item = player_inventory.get_item_at_index(idx);
		auto const randv = util::random::random_vector_float(-16.f, 16.f);
		auto const startpos = item.get_position() + randv;
		item.gravitator.set_position(startpos);
		item.set_offset(sf::Vector2{214.f - 24.f, 62.f - 24.f});
	}
	for (auto& item : vendor.inventory.items) {
		auto const randv = util::random::random_vector_float(-16.f, 16.f);
		auto const startpos = item.get_position() + randv;
		item.gravitator.set_position(startpos);
		item.set_offset(sf::Vector2{214.f - 24.f, 62.f - 24.f});
	}*/
}

} // namespace fornani::gui
