
#include <fornani/entities/player/Player.hpp>
#include <fornani/gui/dialogs/BuilderDialog.hpp>
#include <fornani/service/ServiceProvider.hpp>

namespace fornani::gui {

fornani::gui::BuilderDialog::BuilderDialog(automa::ServiceProvider& svc, world::Map& map, player::Player& player, int vendor_id)
	: IDialog(svc, map, player, vendor_id, "builder"), m_item_sprite{svc, "inventory_items", {18, 18}}, m_orb_display{svc},
	  m_constituents{VendorConstituent{svc, "docket", {{200, 335}, {162, 95}}, 200, util::InterpolationType::cubic}, VendorConstituent{svc, "stage", {{207, 190}, {171, 145}}}, VendorConstituent{svc, "inventory", {{300, 100}, {200, 150}}}},
	  m_zones{InventoryZone{{1, 1}, {19.f, 19.f}, {100.f, 100.f}}, InventoryZone{{1, 2}, {19.f, 19.f}, {200.f, 100.f}}, InventoryZone{{4, 4}, {19.f, 19.f}, {300.f, 100.f}}} {
	auto const& in = svc.data.npc[p_npc_label]["builder"];
	p_upcharge = in["upcharge"].as<float>();
	for (auto const& item : in["docket"].as_array()) { m_docket.push_back(item.as_string()); };
	m_selector.emplace(sf::Vector2i{1, static_cast<int>(m_docket.size())}, sf::Vector2f{19.f, 19.f});
	m_zones.at(BuilderZoneType::docket).table_dimensions.y = static_cast<int>(m_docket.size());
	m_selector->set_lookup({{103, 182}, {20, 20}});
	m_description = std::make_unique<DescriptionGizmo>(svc, map, sf::Vector2f{}, sf::IntRect{}, sf::FloatRect{{308.f, 308.f}, {350.f, 120.f}}, sf::Vector2f{});
	m_description->set_text_only(true);

	std::size_t index = 0;
	for (auto [i, item] : std::views::enumerate(player.catalog.inventory.items_view())) {
		if (!item.item->is_collectible()) { continue; }
		m_player_items.push_back(item.item->get_label());
	}
}

void BuilderDialog::update(automa::ServiceProvider& svc, world::Map& map, player::Player& player, SceneContext& context) {
	IDialog::update(svc, map, player, context);
	if (early_tick_return()) { return; }

	// need to change this to use the InventoryZone, otherwise we easily end up out of range
	auto& controller = svc.input_system;

	if (!m_selector) { return; }
	if (m_item_menu) {
		m_item_menu->handle_inputs(controller, svc.soundboard);
	} else {
		m_zones.set_current_location(m_selector->get_index());
		if (controller.menu_move(input::MoveDirection::up)) {
			svc.soundboard.play_sound("menu_shift");
			if (m_selector->move_direction({0, -1}).up()) {}
		}
		if (controller.menu_move(input::MoveDirection::down)) {
			svc.soundboard.play_sound("menu_shift");
			if (m_selector->move_direction({0, 1}).down()) {}
		}
		if (controller.menu_move(input::MoveDirection::left)) {
			svc.soundboard.play_sound("menu_shift");
			if (m_selector->move_direction({-1, 0}).left()) { switch_zones(-1); }
		}
		if (controller.menu_move(input::MoveDirection::right)) {
			svc.soundboard.play_sound("menu_shift");
			if (m_selector->move_direction({1, 0}).right()) { switch_zones(1); }
		}
		if (svc.input_system.digital(input::DigitalAction::menu_tab_left).triggered) {
			p_state = is_buying() ? DialogState::sell : DialogState::buy;
			svc.soundboard.flags.menu.set(audio::Menu::select);
		}
		if (svc.input_system.digital(input::DigitalAction::menu_tab_right).triggered) {
			p_state = is_buying() ? DialogState::sell : DialogState::buy;
			svc.soundboard.flags.menu.set(audio::Menu::select);
		}
		if (svc.input_system.digital(input::DigitalAction::menu_select).triggered) {
			auto exchange_text = is_buying() ? svc.data.gui_text["exchange_menu"]["build"].as_string() : svc.data.gui_text["exchange_menu"]["build"].as_string();
			m_item_menu.emplace(MiniMenu{svc, {exchange_text, svc.data.gui_text["exchange_menu"]["cancel"].as_string()}, m_selector->get_position(), p_theme});
			svc.soundboard.flags.console.set(audio::Console::menu_open);
		}
		if (svc.input_system.digital(input::DigitalAction::menu_back).triggered) {
			close();
			svc.soundboard.flags.menu.set(audio::Menu::backward_switch);
		}
	}

	auto tag = std::string{};
	if (m_zones.get_zone() == BuilderZoneType::docket) { tag = m_docket.at(m_selector->get_current_selection()); }
	auto const& current_item = svc.data.get_item_json_from_tag(tag);
	if (m_item_menu && m_selector && !current_item.is_null()) {
		if (m_item_menu->was_selected()) {
			switch (m_item_menu->get_selection()) {
			case 0:
				if (player.catalog.inventory.can_build(current_item)) {
					player.catalog.inventory.build_item(current_item);
					player.give_item(current_item["tag"].as_string(), 1);
					svc.soundboard.play_sound("vendor_sale");
					m_item_menu.reset();
				} else {
					svc.soundboard.play_sound("error");
					m_item_menu.reset();
				}
				break;

			case 1:
				m_item_menu.reset();
				svc.soundboard.flags.menu.set(audio::Menu::backward_switch);
				break;
			}
		}
	}

	if (m_item_menu) {
		m_item_menu->update(svc, {1.f, 1.f}, m_selector->get_position());
		if (m_item_menu->was_closed()) { m_item_menu.reset(); }
	}

	if (m_selector) {
		m_selector->set_position(m_zones.current().render_offset + m_selector->get_menu_position());
		m_selector->update();
	}

	m_orb_display.update(player.wallet.get_balance());
	if (m_description) { m_description->update(svc, player, map, {}); }
}

void BuilderDialog::render(automa::ServiceProvider& svc, sf::RenderWindow& win, player::Player& player, world::Map& map, LightShader& shader) {
	IDialog::render(svc, win, player, map, shader);
	if (early_render_return()) { return; }

	if (m_description) { m_description->write(svc, "---", svc.text.fonts.basic); }
	for (auto [index, label] : std::views::enumerate(m_docket)) {
		auto& in_item = svc.data.get_item_json_from_tag(label);
		m_item_sprite.set_channel(in_item["lookup"][0].as<int>());
		m_item_sprite.set_frame(in_item["lookup"][1].as<int>());
		m_item_sprite.set_position(m_zones.at(BuilderZoneType::docket).render_offset + sf::Vector2f{0.f, 38.f} * static_cast<float>(index));
		win.draw(m_item_sprite);
	}
	if (!m_selector) { return; }
	if (m_selector->get_current_selection() >= m_docket.size()) { return; }

	auto& current_item = svc.data.get_item_json_from_tag(m_docket.at(m_selector->get_current_selection()));

	m_orb_display.render(win, {200.f, 480.f});
	if (m_selector->matches(BuilderZoneType::docket)) {
		for (auto [index, ingredient] : std::views::enumerate(current_item["build"]["recipe"].as_array())) {
			auto item = player.catalog.inventory.find_item_stack(ingredient.as_string_view());
			if (item == nullptr) { continue; }
			auto where = m_zones.at(BuilderZoneType::controls).render_offset;
			item->item->render(win, m_item_sprite.get_sprite(), where + sf::Vector2f{0.f, static_cast<float>(index)} * 38.f);
		}
	}
	if (!m_description) { return; }
	m_description->write(svc, current_item["naive_description"].as_string(), svc.text.fonts.basic);
	m_description->render(svc, win, player, shader, p_palette, {600.f, 100.f});

	for (auto [index, id] : std::views::enumerate(m_player_items)) {
		auto item = player.catalog.inventory.find_item_stack(id);
		if (item == nullptr) { continue; }
		if (m_selector->matches(index) && m_description) { m_description->write(svc, item->item->get_description(), svc.text.fonts.basic); }
		auto where = m_zones.at(BuilderZoneType::inventory).render_offset;
		item->item->render(win, m_item_sprite.get_sprite(), where + sf::Vector2f{static_cast<float>(index), 0.f} * 38.f);
	}
	m_selector->render(win, p_selector_sprite.get_sprite(), {2.f, 2.f}, {});
	if (m_item_menu) { m_item_menu->render(win); }

	debug();
}

void BuilderDialog::refresh(automa::ServiceProvider& svc, player::Player& player, world::Map& map) {}

void BuilderDialog::switch_zones(int modulation) {
	m_zones.modulate(modulation);
	auto position = m_selector->get_position();
	auto& current_zone = m_zones.current();
	m_selector.emplace(current_zone.table_dimensions, current_zone.cell_size);
	m_selector->set_position(position, true);
	m_selector->set_lookup({{103, 182}, {20, 20}});
	m_selector->set_selection(m_zones.get_current_location());
}

void BuilderDialog::debug() {
	ImGui::SetNextWindowSize(ImVec2{256.f, 128.f});
	if (ImGui::Begin("Builder Debug")) {
		if (m_selector) {
			ImGui::Text("Selection: %i", m_selector->get_current_selection());
			ImGui::Text("Selector X Index: %i", m_selector->get_index().x);
			ImGui::Text("Selector Y Index: %i", m_selector->get_index().y);
			ImGui::Text("Current Zone: %i", static_cast<int>(m_zones.get_zone()));
		}
		ImGui::End();
	}
}

} // namespace fornani::gui
