
#include <fornani/entities/player/Player.hpp>
#include <fornani/gui/gizmos/LayoutGizmo.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/systems/InputSystem.hpp>

namespace fornani::gui {

LayoutGizmo::LayoutGizmo(automa::ServiceProvider& svc, world::Map& map, player::Player& player, sf::Vector2f placement)
	: Gizmo("Layout", false), m_board{svc, svc.assets.get_texture("layout_board"), {38, 38}, {1, 1}}, m_path{svc.finder, std::filesystem::path{"/data/gui/gizmo_paths.json"}, "layout", 100, util::InterpolationType::quadratic},
	  m_slots{svc, "slot_fixtures", {28, 28}}, m_fixtures{svc, "extension_fixtures", {30, 30}}, m_item_sprite{svc, "inventory_items", constants::i_resolution_vec_padded}, m_player{&player}, m_services{&svc} {
	m_path.set_section("open");
	m_slots.push_and_set_animation("basic", {0, 1, 32, -1});
	m_slots.center();
	m_fixtures.center();
	m_item_sprite.center();
	m_placement = placement;
	p_theme.emplace(svc.data.menu_themes["mini_white"]);

	for (auto& piece : player.catalog.inventory.items_view()) {
		if (piece.item->is_invisible()) { continue; }
		if (!piece.item->is_extension() && !piece.item->is_special()) { continue; }
		m_number_displays.push_back(NumberDisplay(svc, player.catalog.inventory.get_quantity(piece.item->get_label()), piece.item->get_id()));
		if (!piece.item->is_extension()) { continue; }
	}
}

void LayoutGizmo::update(automa::ServiceProvider& svc, player::Player& player, world::Map& map, sf::Vector2f position) {
	Gizmo::update(svc, player, map, position);
	m_slots.tick();
	m_physics.position = get_placement() + position;
	m_path.update();
	m_board.set_position(m_path.get_position() + get_placement());
	m_board.set_dimensions(m_path.get_dimensions());

	// show items
	for (auto& piece : player.catalog.inventory.items_view()) {
		if (piece.item->is_invisible()) { continue; }
		if (piece.item->get_type() != item::ItemType::extension && piece.item->get_type() != item::ItemType::special) { continue; }
	}

	if (m_menu) { m_menu->update(svc, m_slots.get_window_position()); }
}

void LayoutGizmo::render(automa::ServiceProvider& svc, sf::RenderWindow& win, player::Player& player, LightShader& shader, Palette& palette, sf::Vector2f cam, bool foreground) {
	if (is_foreground() != foreground) { return; }
	auto const base = m_board.get_global_center() - cam;
	m_board.render(win, cam);

	// render board and gun layout
	if (m_current_layout != nullptr) {
		auto const half = 18.f;
		auto const offset = half * arms::layout_size_v;
		for (auto [i, slot] : std::views::enumerate(m_current_layout->get_slots())) {
			auto const y = i % 2 == 0 ? -half : half;
			m_slots.set_position(base + sf::Vector2f{static_cast<float>(i) * half * 2.f - offset, y});
			m_slots.set_channel(std::min(2, static_cast<int>(slot.status)));
			if (slot.index && has_flag_set(LayoutGizmoFlags::selected)) {
				if (m_selection.get() == static_cast<int>(*slot.index)) {
					m_slots.set_channel(slot.status == arms::SlotStatus::activated || slot.status == arms::SlotStatus::occupied ? 3 : 2);
					m_slot_index = static_cast<std::size_t>(i);
				}
			}
			if (slot.pin_locations.empty()) { win.draw(m_slots); }
			for (auto const& pin : slot.pin_locations) {
				m_slots.set_frame(static_cast<int>(pin));
				win.draw(m_slots);
			}
			if (slot.status == arms::SlotStatus::activated || slot.status == arms::SlotStatus::occupied) {
				if (slot.active_slot) {
					m_fixtures.set_position(m_slots.get_window_position());
					m_fixtures.set_channel(static_cast<int>(*slot.active_slot));
					win.draw(m_fixtures);
				}
			}
			if (slot.status == arms::SlotStatus::occupied) {
				m_item_sprite.set_position(m_slots.get_window_position());
				auto const id = svc.data.item_id_from_label(*slot.item);
				auto const lookup = sf::Vector2i{svc.data.item[id]["lookup"][0].as<int>(), svc.data.item[id]["lookup"][1].as<int>()} * constants::i_cell_resolution_padded;
				m_item_sprite.set_texture_rect(sf::IntRect{lookup, constants::i_resolution_vec_padded});
				win.draw(m_item_sprite);
			}
		}
	}

	if (debug::is_debug()) { debug(); }

	auto const origin = m_board.get_position() - cam + sf::Vector2f{40.f, 280.f};
	auto constexpr spacing = 40.f;
	auto constexpr selected_offset = 8.f;

	// Fixtures: render only the fixtures available for the selected slot.
	for (std::size_t i = 0; i < m_selectable_fixtures.size(); ++i) {
		auto const pin = m_selectable_fixtures[i];
		auto const selected = has_flag_set(LayoutGizmoFlags::fixture_select_mode) && i == m_fixture_selection;
		auto const where = origin + sf::Vector2f{static_cast<float>(i) * spacing, -130.f - (selected ? selected_offset : 0.f)};

		auto const tag = [&]() -> std::string_view {
			switch (pin) {
			case arms::PinLocation::top_left: return "extension_slot_top_left";
			case arms::PinLocation::top_right: return "extension_slot_top_right";
			case arms::PinLocation::bottom_left: return "extension_slot_bottom_left";
			case arms::PinLocation::bottom_right: return "extension_slot_bottom_right";
			default: return {};
			}
		}();

		if (tag.empty()) { continue; }

		auto const& data = m_services->data.get_item_json_from_tag(tag);
		auto const& lookup = data["lookup"];
		auto const rect = sf::IntRect{sf::Vector2i{lookup[0].as<int>(), lookup[1].as<int>()} * constants::i_cell_resolution_padded, constants::i_resolution_vec_padded};

		auto sprite = m_item_sprite.get_sprite();
		sprite.setTextureRect(rect);
		sprite.setPosition(where);
		win.draw(sprite);
	}

	// items
	// highlight by tag so the display order can differ from the selection order.
	auto display_index = std::size_t{};
	auto const selected_tag = has_flag_set(LayoutGizmoFlags::item_select_mode) && m_item_selection < m_selectable_items.size() ? std::string_view{m_selectable_items[m_item_selection]} : std::string_view{};

	for (auto& piece : player.catalog.inventory.items_view()) {
		if (piece.item->is_invisible() || !piece.item->is_extension()) { continue; }

		auto const& tag = piece.item->get_label();
		auto const selected = !selected_tag.empty() && tag == selected_tag;
		auto const where = origin + sf::Vector2f{static_cast<float>(display_index++) * spacing, -90.f - (selected ? selected_offset : 0.f)};

		piece.item->render(win, m_item_sprite.get_sprite(), where);

		auto const count_offset = sf::Vector2f{0.f, 20.f};
		for (auto& display : m_number_displays) {
			if (display.matches(piece.item->get_id())) { display.render(win, where + count_offset); }
		}
	}

	if (m_menu) { m_menu->render(win); }
}

bool LayoutGizmo::handle_inputs(input::InputSystem& controller, audio::Soundboard& soundboard) {
	if (controller.digital(input::DigitalAction::menu_close).triggered) {
		close_menu();
		exit_fixture_select_mode();
		exit_item_select_mode();
		return true;
	}

	// process menu input
	if (m_menu) {
		m_menu->handle_inputs(controller, soundboard);
		if (m_menu->was_selected()) { handle_menu_selection(soundboard); }
		exit_menu_if_finished();
		return false;
	}

	// back
	if (controller.digital(input::DigitalAction::menu_back).triggered) {
		if (has_flag_set(LayoutGizmoFlags::fixture_select_mode)) {
			exit_fixture_select_mode();
		} else if (has_flag_set(LayoutGizmoFlags::item_select_mode)) {
			exit_item_select_mode();
		} else {
			soundboard.play_sound("menu_back");
			controller.flush_triggers();
			return true;
		}

		soundboard.play_sound("menu_back");
		return false;
	}

	// select
	if (controller.digital(input::DigitalAction::menu_select).triggered) {
		if (has_flag_set(LayoutGizmoFlags::item_select_mode)) {
			if (m_selectable_items.empty()) { return false; }
			m_current_item.emplace(m_selectable_items.at(m_item_selection));
			auto const item_data = m_services->data.get_item_json_from_tag(*m_current_item);
			auto const pin = static_cast<arms::PinLocation>(item_data["pin_location"].as<int>());
			if (m_current_layout->place_item(*m_current_item, pin, m_slot_index)) {
				soundboard.play_sound("pioneer_slot");
				m_current_item.reset();
			} else {
				soundboard.play_sound("error");
				m_services->notifications.push_notification(*m_services, m_services->data.gui_text["notifications"]["mismatched_item_pin"].as_string());
			}

			exit_item_select_mode();
			return false;
		}

		auto const status = m_current_layout->get_slots()[m_slot_index].status;

		switch (status) {
		case arms::SlotStatus::deactivated: handle_deactivated_slot(soundboard); break;
		case arms::SlotStatus::activated:
		case arms::SlotStatus::occupied: open_slot_menu(status); break;
		default: break;
		}

		return false;
	}

	if (controller.menu_move(input::MoveDirection::left, input::DigitalActionQueryType::repeat)) {
		if (has_flag_set(LayoutGizmoFlags::fixture_select_mode)) {
			if (!m_selectable_fixtures.empty()) {
				m_fixture_selection = (m_fixture_selection + m_selectable_fixtures.size() - 1) % m_selectable_fixtures.size();
				soundboard.play_sound("pioneer_tick");
			}
		} else if (has_flag_set(LayoutGizmoFlags::item_select_mode)) {
			if (!m_selectable_items.empty()) {
				m_item_selection = (m_item_selection + m_selectable_items.size() - 1) % m_selectable_items.size();
				soundboard.play_sound("pioneer_tick");
			}
		} else {
			m_selection.modulate(-1);
			soundboard.play_sound("pioneer_pap");
		}
	}

	if (controller.menu_move(input::MoveDirection::right, input::DigitalActionQueryType::repeat)) {
		if (has_flag_set(LayoutGizmoFlags::fixture_select_mode)) {
			if (!m_selectable_fixtures.empty()) {
				m_fixture_selection = (m_fixture_selection + 1) % m_selectable_fixtures.size();
				soundboard.play_sound("pioneer_tick");
			}
		} else if (has_flag_set(LayoutGizmoFlags::item_select_mode)) {
			if (!m_selectable_items.empty()) {
				m_item_selection = (m_item_selection + 1) % m_selectable_items.size();
				soundboard.play_sound("pioneer_tick");
			}
		} else {
			m_selection.modulate(1);
			soundboard.play_sound("pioneer_pap");
		}
	}

	return false;
}

void LayoutGizmo::toggle() { m_path.set_section("toggle"); }

void LayoutGizmo::set_selected(bool const to) {
	if (!has_flag_set(LayoutGizmoFlags::selected) && to) { m_path.set_section("select"); }
	if (has_flag_set(LayoutGizmoFlags::selected) && !to) { m_path.set_section("deselect"); }
	set_flag(LayoutGizmoFlags::selected, to);
}

void LayoutGizmo::set_layout(arms::Weapon& weapon) {
	m_current_layout = &weapon.get_layout();
	m_selection.set_order(m_current_layout->get_number_of_slots());
}

void LayoutGizmo::enter_item_select_mode() {
	m_selectable_items.clear();
	m_item_selection = 0;

	for (auto& piece : m_player->catalog.inventory.items_view()) {
		if (piece.item->is_invisible()) { continue; }
		if (!piece.item->is_extension()) { continue; }
		if (m_current_layout->has_item(piece.item->get_label())) { continue; }

		m_selectable_items.push_back(piece.item->get_label());
	}

	if (m_selectable_items.empty()) {
		m_services->soundboard.play_sound("error");
		m_services->notifications.push_notification(*m_services, m_services->data.gui_text["notifications"]["no_matching_extensions"].as_string());
		return;
	}

	set_flag(LayoutGizmoFlags::item_select_mode);
	m_menu.reset();
}

void LayoutGizmo::enter_fixture_select_mode() {
	m_selectable_fixtures.clear();
	m_fixture_selection = 0;

	auto const& slot = m_current_layout->get_slots()[m_slot_index];
	for (auto const pin : slot.pin_locations) { m_selectable_fixtures.push_back(pin); }

	if (!m_selectable_fixtures.empty()) { set_flag(LayoutGizmoFlags::fixture_select_mode); }
}

void LayoutGizmo::exit_item_select_mode() {
	set_flag(LayoutGizmoFlags::item_select_mode, false);
	m_selectable_items.clear();
	m_item_selection = 0;
	m_current_item.reset();
}

void LayoutGizmo::exit_fixture_select_mode() {
	set_flag(LayoutGizmoFlags::fixture_select_mode, false);
	m_selectable_fixtures.clear();
	m_fixture_selection = 0;
}

void LayoutGizmo::close_menu() { m_menu.reset(); }

void LayoutGizmo::open_slot_menu(arms::SlotStatus status) {
	if (status == arms::SlotStatus::activated) { m_menu.emplace(make_menu("fixure_menu", {"add_item", "swap", "remove", "cancel"})); }
	if (status == arms::SlotStatus::occupied) { m_menu.emplace(make_menu("extension_menu", {"swap", "remove", "cancel"})); }
}

void LayoutGizmo::handle_menu_selection(audio::Soundboard& soundboard) {
	auto const status = m_current_layout->get_slots()[m_slot_index].status;

	if (status == arms::SlotStatus::activated) {
		switch (m_menu->get_selection()) {
		case 0: // add item
			enter_item_select_mode();
			soundboard.play_sound("pioneer_sync");
			break;
		case 1: // swap
			enter_fixture_select_mode();
			soundboard.play_sound("pioneer_open");
			break;
		case 2: // remove
			m_current_layout->remove_fixture(m_slot_index);
			soundboard.play_sound("item_unequip");
			break;
		default: // cancel
			break;
		}
	} else if (status == arms::SlotStatus::occupied) {
		switch (m_menu->get_selection()) {
		case 0: // swap
			enter_item_select_mode();
			soundboard.play_sound("pioneer_sync");
			break;
		case 1: // remove
			m_current_layout->remove_item(m_slot_index);
			soundboard.play_sound("item_unequip");
			break;
		default: // cancel
			break;
		}
	}
}

auto LayoutGizmo::handle_activated_slot(audio::Soundboard& soundboard) -> bool {
	if (!m_current_item) {
		if (!has_flag_set(LayoutGizmoFlags::item_select_mode)) {
			enter_item_select_mode();
			soundboard.play_sound("pioneer_open");
			return true;
		}

		if (!m_selectable_items.empty()) {
			m_current_item.emplace(m_selectable_items[m_item_selection]);
			set_flag(LayoutGizmoFlags::item_select_mode, false);
			m_selectable_items.clear();
		}
		return false;
	}

	auto const item_data = m_services->data.get_item_json_from_tag(*m_current_item);
	auto const item_pin = static_cast<arms::PinLocation>(item_data["pin_location"].as<int>());

	if (m_current_layout->place_item(*m_current_item, item_pin, m_slot_index)) {
		soundboard.play_sound("pioneer_slot");
		m_current_item.reset();
	} else {
		soundboard.play_sound("error");
		m_services->notifications.push_notification(*m_services, m_services->data.gui_text["notifications"]["mismatched_item_pin"].as_string());
	}

	return false;
}

void LayoutGizmo::handle_deactivated_slot(audio::Soundboard& soundboard) {
	if (has_flag_set(LayoutGizmoFlags::fixture_select_mode)) {
		auto const pin = m_selectable_fixtures[m_fixture_selection];
		if (m_current_layout->place_segment(pin, m_slot_index)) {
			soundboard.play_sound("menu_select");
			exit_fixture_select_mode();
		} else {
			soundboard.play_sound("error");
			m_services->notifications.push_notification(*m_services, m_services->data.gui_text["notifications"]["wrong_extension_pin"].as_string());
		}
		return;
	}

	m_selectable_fixtures.clear();

	constexpr std::array fixture_items{std::pair{"extension_slot_top_left", arms::PinLocation::top_left}, std::pair{"extension_slot_top_right", arms::PinLocation::top_right},
									   std::pair{"extension_slot_bottom_left", arms::PinLocation::bottom_left}, std::pair{"extension_slot_bottom_right", arms::PinLocation::bottom_right}};

	auto has_any_fixture = false;
	auto const& slot = m_current_layout->get_slots()[m_slot_index];

	for (auto const& [tag, pin] : fixture_items) {
		if (!m_player->has_item(tag)) { continue; }

		has_any_fixture = true;

		if (std::ranges::find(slot.pin_locations, pin) != slot.pin_locations.end()) { m_selectable_fixtures.push_back(pin); }
	}

	if (!m_selectable_fixtures.empty()) {
		m_fixture_selection = 0;
		enter_fixture_select_mode();
		soundboard.play_sound("map_open");
	} else {
		soundboard.play_sound("error");

		auto const notification = has_any_fixture ? "no_fixture_match" : "no_extension_slots";
		m_services->notifications.push_notification(*m_services, m_services->data.gui_text["notifications"][notification].as_string());
	}
}

void LayoutGizmo::exit_menu_if_finished() {
	if (!m_menu) { return; }
	if (m_menu->was_closed() || (m_menu->was_selected() && m_menu->was_last_option())) { m_menu.reset(); }
}

MiniMenu LayoutGizmo::make_menu(std::string_view key, std::initializer_list<std::string_view> options) {
	auto& gui_text = m_services->data.gui_text;
	std::vector<std::string> labels;
	labels.reserve(options.size());

	for (auto const option : options) { labels.push_back(gui_text[key][option].as_string()); }

	return MiniMenu{*m_services, std::move(labels), m_services->window->f_center_screen(), *p_theme};
}

void LayoutGizmo::debug() {
	static auto sz = ImVec2{440.f, 420.f};
	ImGui::SetNextWindowSize(sz, ImGuiCond_FirstUseEver);

	if (ImGui::Begin("Layout Debug")) {
		ImGui::SeparatorText("Mode");
		ImGui::Text("Fixture Select Mode: %s", has_flag_set(LayoutGizmoFlags::fixture_select_mode) ? "ON" : "");
		ImGui::Text("Item Select Mode: %s", has_flag_set(LayoutGizmoFlags::item_select_mode) ? "ON" : "");
		if (ImGui::CollapsingHeader("Layout Gizmo", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::Text("Fixture select mode: %s", has_flag_set(LayoutGizmoFlags::fixture_select_mode) ? "true" : "false");
			ImGui::Text("Fixture selection: %zu", m_fixture_selection);
			ImGui::Text("Selectable fixtures: %zu", m_selectable_fixtures.size());

			if (!m_selectable_fixtures.empty()) {
				if (m_fixture_selection < m_selectable_fixtures.size()) {
					ImGui::Text("Selected pin: %d", static_cast<int>(m_selectable_fixtures[m_fixture_selection]));
				} else {
					ImGui::TextColored(ImVec4(1.f, 0.f, 0.f, 1.f), "Selection index OUT OF RANGE!");
				}
			}

			ImGui::Separator();
			ImGui::Text("Fixture list:");

			for (std::size_t i = 0; i < m_selectable_fixtures.size(); ++i) {
				auto const pin = m_selectable_fixtures[i];
				auto const selected = i == m_fixture_selection;

				if (selected) {
					ImGui::TextColored(ImVec4(0.f, 1.f, 0.f, 1.f), "> [%zu] pin = %d (selected)", i, static_cast<int>(pin));
				} else {
					ImGui::Text("  [%zu] pin = %d", i, static_cast<int>(pin));
				}
			}
		}
		ImGui::SeparatorText("Selection");
		ImGui::Text("Slot Index: %zu", m_slot_index);
		ImGui::Text("Selection: %i", m_selection.get());
		ImGui::Text("Order: %i", m_selection.get_order());
		ImGui::Text("Slot Count: %i", m_current_layout->get_number_of_slots());
		ImGui::Text("Current Pin: %u", static_cast<unsigned>(m_current_pin));

		ImGui::SeparatorText("Item Selection");
		ImGui::Text("Current Item: %s", m_current_item ? m_current_item->c_str() : "<none>");
		ImGui::Text("Available Items: %zu", m_selectable_items.size());
		if (!m_selectable_items.empty()) {
			ImGui::Text("Item Selection: %zu", m_item_selection);
			ImGui::Text("Highlighted: %s", m_selectable_items[m_item_selection].c_str());
			for (std::size_t i = 0; i < m_selectable_items.size(); ++i) { ImGui::BulletText("%s%s", i == m_item_selection ? "> " : "  ", m_selectable_items[i].c_str()); }
		}

		ImGui::SeparatorText("Slots");
		for (std::size_t i = 0; i < m_current_layout->get_slots().size(); ++i) {
			auto const& slot = m_current_layout->get_slots()[i];
			auto const status = [&]() -> char const* {
				switch (slot.status) {
				case arms::SlotStatus::unavailable: return "unavailable";
				case arms::SlotStatus::deactivated: return "deactivated";
				case arms::SlotStatus::activated: return "activated";
				case arms::SlotStatus::occupied: return "occupied";
				}
				return "unknown";
			}();

			if (ImGui::TreeNode((void*)(std::uintptr_t)i, "Slot %zu [%s]", i, status)) {
				ImGui::Text("Logical Index: %s", slot.index ? std::to_string(*slot.index).c_str() : "<none>");
				ImGui::Text("Active Pin: %s", slot.active_slot ? std::to_string(static_cast<int>(*slot.active_slot)).c_str() : "<none>");
				ImGui::Text("Item: %s", slot.item ? slot.item->c_str() : "<none>");

				ImGui::Text("Allowed Pins:");
				for (auto const pin : slot.pin_locations) {
					ImGui::SameLine();
					ImGui::Text("%u", static_cast<unsigned>(pin));
				}

				ImGui::TreePop();
			}
		}

		ImGui::SeparatorText("Positions");
		ImGui::Text("Path: (%.1f, %.1f)", m_path.get_position().x, m_path.get_position().y);
		ImGui::Text("Board: (%.1f, %.1f)", m_board.get_global_center().x, m_board.get_global_center().y);
	}
	ImGui::End();
}

} // namespace fornani::gui
