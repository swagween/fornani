
#include "fornani/gui/VendorDialog.hpp"
#include <algorithm>
#include <cctype>
#include "fornani/entities/player/Player.hpp"
#include "fornani/graphics/Transition.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/utils/ColorUtils.hpp"
#include "fornani/utils/Math.hpp"
#include "fornani/utils/Random.hpp"
#include "fornani/world/Map.hpp"

namespace fornani::gui {

VendorDialog::VendorDialog(automa::ServiceProvider& svc, world::Map& map, player::Player& player, int vendor_id)
	: vendor_id(vendor_id), m_buy_selector{{8, 4}, {18.f, 18.f}}, m_sell_selector{{2, 1}}, orb{.sprite{anim::AnimatedSprite(svc.assets.get_texture("orbs"), {24, 24})}}, m_artwork{svc, "vendor_background"},
	  text{.vendor_name{svc.text.fonts.title},
		   .buy_tab{svc.text.fonts.title},
		   .sell_tab{svc.text.fonts.title},
		   .orb_count{svc.text.fonts.title},
		   .price{svc.text.fonts.title},
		   .price_number{svc.text.fonts.title},
		   .item_label{svc.text.fonts.title}},
	  m_constituents{
		  VendorConstituent{svc, "portrait", {{}, {96, 144}}, 64},		VendorConstituent{svc, "wares", {{0, 335}, {200, 118}}, 200, util::InterpolationType::cubic},
		  VendorConstituent{svc, "description", {{96, 0}, {256, 137}}}, VendorConstituent{svc, "name", {{0, 144}, {103, 58}}, 32},
		  VendorConstituent{svc, "core", {{0, 202}, {207, 133}}},		VendorConstituent{svc, "selection", {{200, 335}, {162, 95}}},
		  VendorConstituent{svc, "nani", {{207, 190}, {171, 145}}, 64},
	  },
	  m_intro{300}, m_vendor_portrait{svc, "character_portraits"}, m_orb_display{svc}, m_selector_sprite{svc, "vendor_gizmo"},
	  my_npc{*std::find_if(map.get_entities<NPC>().begin(), map.get_entities<NPC>().end(), [vendor_id](auto const& n) { return n->get_vendor_id() == vendor_id; })}, npc_id{vendor_id}, m_item_sprite{svc, "inventory_items"} {
	flags.set(VendorDialogStatus::opened);
	// m_artwork.set_texture_rect(sf::IntRect{{0, (vendor_id - 1) * (svc.window->i_screen_dimensions().y / constants::i_scale_factor)}, {svc.window->i_screen_dimensions() / constants::i_scale_factor}});
	m_artwork.center();
	m_artwork.set_position(svc.window->f_center_screen());

	for (auto& in_anim = svc.data.drop["orb"]["animation"]; auto& param : in_anim["params"].as_array()) {
		anim::Parameters a{};
		a.duration = param["duration"].as<int>();
		a.framerate = param["framerate"].as<int>();
		a.num_loops = param["num_loops"].as<int>();
		orb.sprite.push_params(param["label"].as_string(), a);
	}
	orb.sprite.set_params("neutral", true);

	// text
	auto lbl = svc.data.get_npc_label_from_id(npc_id);
	if (lbl) {
		std::string str = lbl->data();
		std::ranges::transform(str, str.begin(), [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
		text.vendor_name.setString(str);
	}
	text.buy_tab.setString(svc.data.gui_text["vendor"]["buy"].as_string());
	text.sell_tab.setString(svc.data.gui_text["vendor"]["sell"].as_string());
	text.price.setString(svc.data.gui_text["vendor"]["price"].as_string());
	// font
	text.vendor_name.setCharacterSize(16);
	text.buy_tab.setCharacterSize(16);
	text.sell_tab.setCharacterSize(16);
	text.price.setCharacterSize(16);
	text.item_label.setCharacterSize(16);
	text.price_number.setCharacterSize(16);
	// colors
	text.vendor_name.setFillColor(colors::pioneer_red);
	text.price.setFillColor(colors::pioneer_red);
	text.item_label.setFillColor(colors::pioneer_red);
	text.price_number.setFillColor(colors::periwinkle);
	// origins
	text.buy_tab.setOrigin({0.f, text.buy_tab.getLocalBounds().getCenter().y});
	text.sell_tab.setOrigin({0.f, text.sell_tab.getLocalBounds().getCenter().y});
	text.price.setOrigin({0.f, text.price.getLocalBounds().getCenter().y});

	m_vendor_portrait.set_texture_rect(sf::IntRect{{npc_id * 64, 0}, {64, 128}});
	m_buy_selector.set_lookup({{103, 182}, {20, 20}});

	// background color
	m_background.setFillColor(colors::pioneer_black);
	m_background.setSize(svc.window->f_screen_dimensions());

	refresh(player, map);
	m_intro.start();
	for (auto& c : m_constituents) { c.update(); }
	util::ColorUtils::reset();

	m_description = std::make_unique<DescriptionGizmo>(svc, map, sf::Vector2f{}, sf::IntRect{}, sf::FloatRect{{360.f, 60.f}, {200.f, 200.f}}, sf::Vector2f{});
	m_description->set_text_only(true);

	svc.music_player.filter_fade_in(80.f, 40.f);
	svc.ambience_player.set_balance(1.f);

	// initialize item list
	for (auto& row : m_items_list) {
		for (auto& slot : row) { slot = -1; }
	}
	auto vendor = my_npc->get_vendor();
	if (vendor) {
		for (auto [i, item] : std::views::enumerate(vendor.value()->inventory.items_view())) {
			auto rarity = static_cast<int>(item->get_rarity());
			if (rarity >= m_items_list.size()) { continue; }
			for (auto& slot : m_items_list[rarity]) {
				if (slot == -1) {
					slot = item->get_id();
					break;
				} // populate next slot with item
			}
		}
	}
	for (auto [i, row] : std::views::enumerate(m_items_list)) { NANI_LOG_INFO(m_logger, "Row {}: {}", i, row); }

	NANI_LOG_INFO(m_logger, "Vendor NPC: {}", my_npc->get_tag());
}

void VendorDialog::update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {

	m_intro.update();
	if (is_opening()) { return; }

	auto& controller = svc.controller_map;
	if (controller.digital_action_status(config::DigitalAction::menu_up).triggered) {
		m_description->flush();
		if (m_buy_selector.move_direction({0, -1}).up()) {}
		svc.soundboard.flags.menu.set(audio::Menu::shift);
	}
	if (controller.digital_action_status(config::DigitalAction::menu_down).triggered) {
		m_description->flush();
		if (m_buy_selector.move_direction({0, 1}).down()) {}
		svc.soundboard.flags.menu.set(audio::Menu::shift);
	}
	if (controller.digital_action_status(config::DigitalAction::menu_left).triggered) {
		m_description->flush();
		if (m_buy_selector.move_direction({-1, 0}).left()) {}
		svc.soundboard.flags.menu.set(audio::Menu::shift);
	}
	if (controller.digital_action_status(config::DigitalAction::menu_right).triggered) {
		m_description->flush();
		if (m_buy_selector.move_direction({1, 0}).right()) {}
		svc.soundboard.flags.menu.set(audio::Menu::shift);
	}

	m_orb_display.update(player.wallet.get_balance());
	if (m_description) { m_description->update(svc, player, map, m_constituents[static_cast<int>(VendorConstituentType::description)].get_window_position() + sf::Vector2f{108.f, 108.f}); }

	!flags.test(VendorDialogStatus::opened) ? m_background.setFillColor(util::ColorUtils::fade_out(colors::pioneer_black)) : m_background.setFillColor(util::ColorUtils::fade_in(colors::pioneer_black));

	if (svc.controller_map.digital_action_status(config::DigitalAction::menu_tab_left).triggered) {
		m_state = is_buying() ? VendorState::sell : VendorState::buy;
		svc.soundboard.flags.menu.set(audio::Menu::select);
	}
	if (svc.controller_map.digital_action_status(config::DigitalAction::menu_tab_right).triggered) {
		m_state = is_buying() ? VendorState::sell : VendorState::buy;
		svc.soundboard.flags.menu.set(audio::Menu::select);
	}
	if (svc.controller_map.digital_action_status(config::DigitalAction::menu_cancel).triggered) {
		close(svc);
		svc.soundboard.flags.menu.set(audio::Menu::backward_switch);
	}

	for (auto& c : m_constituents) { c.update(); }

	is_buying() ? m_constituents[static_cast<int>(VendorConstituentType::selection)].set_texture_rect({{200, 335}, {81, 95}})
				: m_constituents[static_cast<int>(VendorConstituentType::selection)].set_texture_rect({{281, 335}, {81, 95}}); // selection

	auto selector_offset = m_buy_selector.get_menu_position() + sf::Vector2f{16.f, 28.f};
	m_buy_selector.set_position(m_constituents[static_cast<int>(VendorConstituentType::wares)].get_window_position() + selector_offset);
	m_buy_selector.update();

	// update text
	is_buying() ? text.buy_tab.setFillColor(colors::pioneer_red) : text.buy_tab.setFillColor(colors::pioneer_dark_red);
	is_selling() ? text.sell_tab.setFillColor(colors::pioneer_red) : text.sell_tab.setFillColor(colors::pioneer_dark_red);
	auto bx_off = is_selling() ? 8.f : 0.f;
	auto sx_off = is_buying() ? 8.f : 0.f;
	text.buy_tab.setPosition(m_constituents[static_cast<int>(VendorConstituentType::selection)].get_window_position() + sf::Vector2f{50.f + bx_off, 52.f});
	text.sell_tab.setPosition(m_constituents[static_cast<int>(VendorConstituentType::selection)].get_window_position() + sf::Vector2f{50.f + sx_off, 142.f});
	text.vendor_name.setPosition(m_constituents[static_cast<int>(VendorConstituentType::name)].get_window_position() + sf::Vector2f{26.f, 34.f});
	text.price.setPosition(m_constituents[static_cast<int>(VendorConstituentType::core)].get_window_position() + sf::Vector2f{72.f, 164.f});

	// vendor stuff
	if (!my_npc) {
		NANI_LOG_WARN(m_logger, "Tried to update a vendor with invalid NPC pointer!");
		return;
	}
	auto vendor = my_npc->get_vendor();
	if (vendor) {
		for (auto& item : vendor.value()->inventory.items_view()) {
			// info.load_single_message(item.get_description());
			auto f_value = static_cast<float>(item->get_value());
			sale_price = f_value + f_value * vendor.value()->get_upcharge();
			text.price_number.setString(std::format("{}", sale_price));
		}
	}
}

void VendorDialog::render(automa::ServiceProvider& svc, sf::RenderWindow& win, player::Player& player, world::Map& map) {
	win.draw(m_artwork);
	if (is_opening()) { return; }
	win.draw(m_background);
	auto nani = player.wardrobe_widget.get_sprite();
	nani.setPosition(m_constituents[static_cast<int>(VendorConstituentType::nani)].get_window_position() + sf::Vector2f{196.f, 14.f});
	win.draw(nani);
	m_vendor_portrait.set_position(m_constituents[static_cast<int>(VendorConstituentType::portrait)].get_window_position() + sf::Vector2f{20.f, 10.f});
	win.draw(m_vendor_portrait);
	for (auto& c : m_constituents) { c.render(win); }
	m_orb_display.render(win, m_constituents[static_cast<int>(VendorConstituentType::nani)].get_window_position() + sf::Vector2f{24.f, 238.f});
	m_buy_selector.render(win, m_selector_sprite.get_sprite(), {}, {});

	auto vendor = my_npc->get_vendor();
	/*if (vendor) {
		for (auto [i, item] : std::views::enumerate(vendor.value()->inventory.items_view())) {
			auto where = m_constituents[static_cast<int>(VendorConstituentType::wares)].get_window_position() + sf::Vector2f{16.f, 28.f};
			item->render(win, m_item_sprite.get_sprite(), where + m_buy_selector.get_table_position_from_index(i) * constants::f_scale_factor);
		}
	}*/
	for (auto [i, row] : std::views::enumerate(m_items_list)) {
		for (auto [j, slot] : std::views::enumerate(row)) {
			if (vendor) {
				for (auto [k, item] : std::views::enumerate(vendor.value()->inventory.items_view())) {
					if (slot == item->get_id()) {
						auto where = m_constituents[static_cast<int>(VendorConstituentType::wares)].get_window_position() + sf::Vector2f{16.f, 28.f};
						item->render(win, m_item_sprite.get_sprite(), where + m_buy_selector.get_table_position_from_index(j + i * 8) * constants::f_scale_factor);
					}
				}
			}
		}
	}

	win.draw(text.buy_tab);
	win.draw(text.sell_tab);
	win.draw(text.vendor_name);
	win.draw(text.price);
}

void VendorDialog::close(automa::ServiceProvider& svc) {
	flags.reset(VendorDialogStatus::opened);
	svc.music_player.filter_fade_out();
}

void VendorDialog::update_table(player::Player& player, world::Map& map, bool new_dim) {}

void VendorDialog::refresh(player::Player& player, world::Map& map) const {}

VendorConstituent::VendorConstituent(automa::ServiceProvider& svc, std::string_view label, sf::IntRect lookup, int speed, util::InterpolationType type)
	: Drawable(svc, "vendor_gizmo"), path{svc.finder, std::filesystem::path{"/data/gui/gizmo_paths.json"}, "vendor_" + std::string{label}, speed, type} {
	set_texture_rect(lookup);
}

void VendorConstituent::update() {
	path.update();
	set_position(path.get_position());
}

void VendorConstituent::render(sf::RenderWindow& win) { win.draw(*this); }

} // namespace fornani::gui
