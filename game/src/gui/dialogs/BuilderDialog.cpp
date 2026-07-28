
#include <fornani/entities/player/Player.hpp>
#include <fornani/graphics/Color.hpp>
#include <fornani/gui/dialogs/BuilderDialog.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <span>

namespace fornani::gui {

fornani::gui::BuilderDialog::BuilderDialog(automa::ServiceProvider& svc, world::Map& map, player::Player& player, int vendor_id)
	: IDialog(svc, map, player, vendor_id, "builder"), m_item_sprite{svc, "inventory_items", {18, 18}},
	  m_constituents{VendorConstituent{svc, "docket", {{200, 335}, {162, 95}}, 200, util::InterpolationType::cubic}, VendorConstituent{svc, "stage", {{207, 190}, {171, 145}}}, VendorConstituent{svc, "inventory", {{300, 100}, {200, 150}}}},
	  m_zones{InventoryZone{{1, 1}, {19.f, 19.f}, sf::Vector2f{388.f, 78.f} * constants::f_scale_factor}, InventoryZone{{3, 1}, {16.f, 16.f}, sf::Vector2f{119.f, 132.f} * constants::f_scale_factor},
			  InventoryZone{{1, 1}, {19.f, 19.f}, sf::Vector2f{116.f, 18.f} * constants::f_scale_factor}},
	  m_unknown{svc, "unknown_item", {18, 18}}, m_turntable{svc, "item_turntable", {128, 128}}, m_flat_shader{svc.finder}, m_question_mark{svc, "question_mark", {32, 32}}, m_just_built{200}, m_shaker{{16.f, 24, 620}},
	  m_backdrop{svc, "builder_backdrop"}, m_mask{svc, "builder_mask"}, m_overlay{svc, "builder_overlay"}, m_press{svc, "builder_press"}, m_dot{svc, "pioneer_dot"} {
	auto const& in = svc.data.npc[p_npc_label]["builder"];
	p_upcharge = in["upcharge"].as<float>();
	for (auto const& item : in["docket"].as_array()) { m_docket.push_back(item.as_string()); };
	m_selector.emplace(sf::Vector2i{1, static_cast<int>(m_docket.size())}, sf::Vector2f{19.f, 19.f});
	m_zones.at(BuilderZoneType::docket).table_dimensions.y = static_cast<int>(m_docket.size());
	m_selector->set_lookup({{103, 182}, {20, 20}});
	m_description = std::make_unique<DescriptionGizmo>(svc, map, sf::Vector2f{}, sf::IntRect{}, sf::FloatRect{sf::Vector2f{225.f, 198.f} * constants::f_scale_factor, {324.f, 96.f}}, sf::Vector2f{});
	m_description->set_text_only(true);
	m_turntable.push_and_set_animation("turntable", {0, 32, 20, -1});
	m_question_mark.push_and_set_animation("basic", {0, 6, 16, -1});
	m_turntable.center();
	m_question_mark.center();
	m_dot.center();
	m_shaker.set_point({});

	for (auto [i, item] : std::views::enumerate(player.catalog.inventory.items_view())) {
		if (!item.item->is_ingredient()) { continue; }
		m_player_items.push_back(item.item->get_label());
		m_number_displays.push_back(NumberDisplay(svc, player.catalog.inventory.get_quantity(item.item->get_label()), item.item->get_id()));
	}
	m_zones.at(BuilderZoneType::inventory).table_dimensions.x = static_cast<int>(m_player_items.size());
}

void BuilderDialog::update(automa::ServiceProvider& svc, world::Map& map, player::Player& player, SceneContext& context) {
	IDialog::update(svc, map, player, context);
	if (early_tick_return()) { return; }

	m_turntable.tick();
	m_question_mark.tick();
	m_just_built.update();
	m_shaker.tick_static(p_position);

	m_turntable.set_framerate(20 - static_cast<int>(19.f * m_just_built.get_normalized()));

	// need to change this to use the InventoryZone, otherwise we easily end up out of range
	auto& controller = svc.input_system;

	if (!m_selector) { return; }
	if (m_item_menu) {
		m_item_menu->handle_inputs(controller, svc.soundboard);
	} else {
		m_zones.set_current_location(m_selector->get_index());
		if (controller.menu_move(input::MoveDirection::up)) {
			refresh_stage(svc);
			svc.soundboard.play_sound("menu_shift");
			if (m_selector->move_direction({0, -1}).up()) {}
		}
		if (controller.menu_move(input::MoveDirection::down)) {
			refresh_stage(svc);
			svc.soundboard.play_sound("menu_shift");
			if (m_selector->move_direction({0, 1}).down()) {}
		}
		if (controller.menu_move(input::MoveDirection::left)) {
			refresh_stage(svc);
			svc.soundboard.play_sound("menu_shift");
			if (m_selector->move_direction({-1, 0}).left()) { switch_zones(-1); }
		}
		if (controller.menu_move(input::MoveDirection::right)) {
			refresh_stage(svc);
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
			if (m_zones.get_zone() == BuilderZoneType::docket) {
				auto exchange_text = is_buying() ? svc.data.gui_text["exchange_menu"]["build"].as_string() : svc.data.gui_text["exchange_menu"]["build"].as_string();
				m_item_menu.emplace(MiniMenu{svc, {exchange_text, svc.data.gui_text["exchange_menu"]["cancel"].as_string()}, m_selector->get_position(), p_theme});
				svc.soundboard.flags.console.set(audio::Console::menu_open);
			} else {
				svc.soundboard.play_sound("error");
			}
		}
		if (svc.input_system.digital(input::DigitalAction::menu_back).triggered) {
			close();
			svc.soundboard.flags.menu.set(audio::Menu::backward_switch);
		}
	}

	auto tag = std::string{};
	if (m_zones.get_zone() == BuilderZoneType::docket) { tag = m_docket.at(m_selector->get_current_selection()); }
	if (m_zones.get_zone() == BuilderZoneType::inventory) { tag = m_player_items.at(m_selector->get_current_selection()); }
	auto const& current_item = svc.data.get_item_json_from_tag(tag);
	p_sale_price = -current_item["build"]["craft_price"].as<float>();
	if (m_item_menu && m_selector && !current_item.is_null()) {
		if (m_item_menu->was_selected()) {
			switch (m_item_menu->get_selection()) {
			case 0:
				build_item(svc, player, current_item);
				m_item_menu.reset();
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
		m_selector->set_position(m_zones.current().render_offset + m_selector->get_menu_position() + p_position);
		m_selector->update();
	}

	if (m_description) { m_description->update(svc, player, map, p_position); }
}

void BuilderDialog::render(automa::ServiceProvider& svc, sf::RenderWindow& win, player::Player& player, world::Map& map, LightShader& shader, Renderer& renderer) {
	IDialog::render(svc, win, player, map, shader, renderer);
	if (early_render_return()) { return; }

	m_backdrop.set_position(p_position);
	win.draw(m_backdrop);

	auto const& docket_item = svc.data.get_item_json_from_tag(m_docket_item);

	// draw stage
	if (!docket_item.is_null()) {
		for (auto [index, ingredient] : std::views::enumerate(docket_item["build"]["recipe"].as_array())) {
			auto item = player.catalog.inventory.find_item_stack(ingredient.as_string_view());
			auto offset = sf::Vector2f{static_cast<float>(index), 0.f} * 34.f;
			auto where = m_zones.at(BuilderZoneType::stage).render_offset + offset + p_position;
			if (item == nullptr) {
				m_unknown.set_position(where);
				m_unknown.set_channel(0);
				if (player.catalog.inventory.was_item_logged(ingredient.as_string_view())) { m_unknown.set_channel(1); }
				win.draw(m_unknown);
				continue;
			}
			item->item->render(win, m_item_sprite.get_sprite(), where);
		}
	}

	auto press_pos = sf::Vector2f{47.f, 126.f};
	press_pos.x += 52.f * m_just_built.get_quadratic_normalized();
	m_press.set_position(press_pos * constants::f_scale_factor + p_position);
	win.draw(m_press);
	m_mask.set_position(p_position);
	win.draw(m_mask);

	sf::Color highlight(245, 195, 135); // warm amber glow
	sf::Color shadow(55, 32, 18);		// softened warm dark
	p_holo_shader.finalize(svc.ticker.total_seconds_passed.count(), highlight, shadow, 1.1f);
	player.wardrobe_widget.set_position(sf::Vector2f{13.f, 58.f} * constants::f_scale_factor + p_position);
	player.wardrobe_widget.submit(svc, win, p_holo_shader, {}, highlight, shadow);
	p_vendor_portrait.set_position(sf::Vector2f{416.f, 57.f} * constants::f_scale_factor + p_position);
	p_holo_shader.submit(win, p_vendor_portrait.get_sprite());

	if (!m_selector) { return; }

	// get current item
	auto tag = std::string{};
	if (m_zones.get_zone() == BuilderZoneType::docket) {
		m_docket_item = m_docket.at(m_selector->get_current_selection());
		tag = m_docket_item;
	}
	if (m_zones.get_zone() == BuilderZoneType::inventory) { tag = m_player_items.at(m_selector->get_current_selection()); }
	if (m_zones.get_zone() == BuilderZoneType::stage) { tag = m_staged_items.at(m_selector->get_current_selection()); }
	auto const& current_item = svc.data.get_item_json_from_tag(tag);

	p_price_display.render(win, sf::Vector2f{110.f, 180.f} * constants::f_scale_factor + p_position);
	p_orb_display.render(win, sf::Vector2f{7.f, 204.f} * constants::f_scale_factor + p_position);

	// draw inventory
	auto count_offset = sf::Vector2f{32.f, 38.f};
	for (auto [index, id] : std::views::enumerate(m_player_items)) {
		auto item = player.catalog.inventory.find_item_stack(id);
		if (item == nullptr) { continue; }
		if (m_selector->matches(index) && m_description) { m_description->write(svc, item->item->get_description(), svc.text.fonts.basic); }
		auto where = m_zones.at(BuilderZoneType::inventory).render_offset + sf::Vector2f{static_cast<float>(index), 0.f} * 38.f + p_position;
		item->item->render(win, m_item_sprite.get_sprite(), where);
		for (auto& display : m_number_displays) {
			if (display.matches(item->item->get_id())) { display.render(win, where + count_offset); }
		}
	}

	// turntable
	m_turntable.set_position(sf::Vector2f{289.f, 126.f} * constants::f_scale_factor + p_position);
	m_question_mark.set_position(m_turntable.get_window_position());
	m_turntable.set_channel(svc.data.get_item_json_from_tag(m_docket_item)["build"]["lookup"].as<int>());
	auto color = colors::pioneer_black;
	if (m_just_built.running()) {
		auto o = colors::orange;
		o.a = 100;
		auto r = colors::red;
		r.a = 60;
		auto cvec = std::vector{colors::ui_white, o, r, colors::transparent};
		color = gradient_color(cvec, m_just_built.get_inverse_normalized());
	}
	m_flat_shader.finalize(color);
	auto hide_me = false;
	if (!player.has_item(m_docket_item) || m_just_built.running()) {
		hide_me = true;
	} else {
		auto item = player.catalog.inventory.find_item_stack(m_docket_item);
		if (item != nullptr) {
			if (m_docket_item == "equip_slot") {
				if (item->quantity < 1) { hide_me = true; }
			}
		}
	}
	hide_me ? m_flat_shader.submit(win, m_turntable.get_sprite()) : win.draw(m_turntable);
	if (hide_me && !m_just_built.running()) { win.draw(m_question_mark); }

	// draw docket
	if (m_description) { m_description->write(svc, "---", svc.text.fonts.basic); }
	for (auto [index, label] : std::views::enumerate(m_docket)) {
		auto& in_item = svc.data.get_item_json_from_tag(label);
		m_item_sprite.set_channel(in_item["lookup"][0].as<int>());
		m_item_sprite.set_frame(in_item["lookup"][1].as<int>());
		m_item_sprite.set_position(m_zones.at(BuilderZoneType::docket).render_offset + sf::Vector2f{0.f, 38.f} * static_cast<float>(index) + p_position);
		win.draw(m_item_sprite);
		auto light_on = player.has_item(label);
		auto item = player.catalog.inventory.find_item_stack(label);
		if (item != nullptr) {
			if (label == "equip_slot") {
				if (item->quantity < svc.data.get_item_json_from_tag(label)["stack_limit"].as<int>()) { light_on = false; }
			}
		}
		if (light_on) {
			m_dot.set_position(sf::Vector2f{376.f, 89.f} * constants::f_scale_factor + sf::Vector2f{0.f, index * 36.f} + p_position + sf::Vector2f{-1.f, -1.f});
			win.draw(m_dot);
		}
	}

	if (m_description) {
		m_description->write(svc, current_item["naive_description"].as_string(), svc.text.fonts.basic);
		m_description->render(svc, win, player, shader, p_palette, {});
	}
	m_overlay.set_position(p_position);
	win.draw(m_overlay);
	m_selector->render(win, p_selector_sprite.get_sprite(), {2.f, 2.f}, {});

	if (m_item_menu) { m_item_menu->render(win); }
	IDialog::post_render(svc, win, renderer);
	// debug();
}

void BuilderDialog::refresh(automa::ServiceProvider& svc, player::Player& player, world::Map& map) {}

void BuilderDialog::build_item(automa::ServiceProvider& svc, player::Player& player, dj::Json const& current_item) {
	auto item = player.catalog.inventory.find_item_stack(m_docket_item);
	if (item != nullptr) {
		if (item->quantity == current_item["stack_limit"].as<int>() || item->item->is_unique()) {
			svc.notifications.push_notification(svc, svc.data.gui_text["notifications"]["already_have_max_item"].as_string());
			return;
		}
	}
	if (!player.catalog.inventory.can_build(current_item)) {
		svc.soundboard.play_sound("error");
		svc.notifications.push_notification(svc, svc.data.gui_text["notifications"]["cannot_build"].as_string());
		return;
	}
	for (auto [index, id] : std::views::enumerate(m_player_items)) {
		auto where = m_zones.at(BuilderZoneType::inventory).render_offset + sf::Vector2f{static_cast<float>(index), 0.f} * 38.f + sf::Vector2f{18.f, 18.f};
		auto const& recipe = current_item["build"]["recipe"];
		for (auto const& ingredient : recipe.as_array()) {
			if (id == ingredient.as_string()) { spawn_effect(svc, "bonus_heart", where, {}, 2); }
		}
	}
	spawn_emitter(svc, "radiance", m_turntable.get_window_position() - sf::Vector2f{64.f, 64.f}, Direction{}, {128.f, 128.f});
	// spawn_effect(svc, "giga_flare", m_turntable.get_window_position());
	player.catalog.inventory.build_item(current_item);
	player.give_item(current_item["tag"].as_string(), 1);
	svc.soundboard.play_sound("vendor_sale");
	svc.soundboard.play_sound("pioneer_boot");
	svc.soundboard.play_sound("pioneer_sync");
	// svc.soundboard.play_sound("laser1");
	player.give_drop(item::DropType::orb, p_sale_price);
	p_orb_indicator.add(p_sale_price);
	m_player_items.clear();
	m_number_displays.clear();
	for (auto [i, item] : std::views::enumerate(player.catalog.inventory.items_view())) {
		if (!item.item->is_ingredient()) { continue; }
		m_player_items.push_back(item.item->get_label());
		m_number_displays.push_back(NumberDisplay(svc, player.catalog.inventory.get_quantity(item.item->get_label()), item.item->get_id()));
	}
	m_zones.at(BuilderZoneType::inventory).table_dimensions.x = static_cast<int>(m_player_items.size());
	m_just_built.start();
	m_shaker.shake();
	svc.ticker.freeze_frame(0.1f, 0.04f);
}

void BuilderDialog::switch_zones(int modulation) {
	m_zones.modulate(modulation);
	auto position = m_selector->get_position();
	auto& current_zone = m_zones.current();
	m_selector.emplace(current_zone.table_dimensions, current_zone.cell_size);
	m_selector->set_position(position, true);
	m_selector->set_lookup({{103, 182}, {20, 20}});
	m_selector->set_selection(m_zones.get_current_location());
}

void BuilderDialog::refresh_stage(automa::ServiceProvider& svc) {
	m_staged_items.clear();
	auto const& docket_item = svc.data.get_item_json_from_tag(m_docket_item);
	auto const& recipe = docket_item["build"]["recipe"];
	for (auto const& ingredient : recipe.as_array()) { m_staged_items.push_back(ingredient.as_string()); }
}

void BuilderDialog::debug() {
	ImGui::SetNextWindowSize(ImVec2{256.f, 128.f});
	if (ImGui::Begin("Builder Debug")) {
		if (m_selector) {
			ImGui::Text("Selection: %i", m_selector->get_current_selection());
			ImGui::Text("Selector X Index: %i", m_selector->get_index().x);
			ImGui::Text("Selector Y Index: %i", m_selector->get_index().y);
			ImGui::Text("Current Zone: %i", static_cast<int>(m_zones.get_zone()));
			ImGui::Text("Player Inventory: %i", static_cast<int>(m_player_items.size()));
		}
		ImGui::End();
	}
}

} // namespace fornani::gui
