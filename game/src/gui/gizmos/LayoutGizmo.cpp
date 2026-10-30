
#include <fornani/entities/player/Player.hpp>
#include <fornani/gui/gizmos/LayoutGizmo.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/systems/InputSystem.hpp>

namespace fornani::gui {

auto constexpr selected_fixture_offset_v = 4.f;

LayoutGizmo::LayoutGizmo(automa::ServiceProvider& svc, world::Map& map, player::Player& player, sf::Vector2f placement)
	: Gizmo("Layout", false), m_board{svc, svc.assets.get_texture("layout_board"), {38, 38}, {1, 1}}, m_path{svc.finder, std::filesystem::path{"/data/gui/gizmo_paths.json"}, "layout", 100, util::InterpolationType::quadratic},
	  m_slots{svc, "fixture_slots", {24, 24}}, m_fixtures{svc, "fixture_plate", {36, 36}}, m_item_sprite{svc, "inventory_items", constants::i_resolution_vec_padded}, m_player{&player}, m_services{&svc} {
	m_path.set_section("open");
	m_slots.push_and_set_animation("basic", {0, 1, 32, -1});
	m_slots.center();
	m_fixtures.center();
	m_item_sprite.center();
	m_placement = placement;
	p_theme.emplace(svc.data.menu_themes["mini_white"]);

	refresh();
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
	auto const dt = m_animation_clock.restart().asSeconds();
	update_item_flight(dt);
}

void LayoutGizmo::render(automa::ServiceProvider& svc, sf::RenderWindow& win, player::Player& player, LightShader& shader, Palette& palette, sf::Vector2f cam, bool foreground) {
	if (is_foreground() != foreground) { return; }
	m_cam = cam;
	auto const base = m_board.get_global_center() - cam;
	m_board.render(win, cam);

	// render board and gun layout
	if (m_current_layout != nullptr) {
		auto const half = 20.f;
		auto const offset = half * arms::layout_size_v;

		for (auto [i, slot] : std::views::enumerate(m_current_layout->get_slots())) {
			auto position = get_slot_position(i, cam);
			auto const y = i % 2 == 0 ? -half : half;
			auto is_selected = false;
			if (slot.index && has_flag_set(LayoutGizmoFlags::selected)) { is_selected = m_selection.get() == static_cast<int>(*slot.index); }
			if (is_selected) { position.y -= 4.f; }

			// fixture plate - always render, regardless of layout or slot status
			m_fixtures.set_position(position);
			m_fixtures.set_channel(0);
			if (is_selected) { m_fixtures.set_channel(1); }
			win.draw(m_fixtures);

			if (slot.status == arms::SlotStatus::unavailable) { continue; }

			// slot pins
			m_slots.set_position(position);

			// base slot (channel 0)
			m_slots.set_channel(0);
			m_slots.set_frame(0);
			win.draw(m_slots);

			// selected slot highlight (channel 0, frame 1)
			if (slot.index && has_flag_set(LayoutGizmoFlags::selected)) {
				if (is_selected) {
					m_slots.set_frame(1);
					win.draw(m_slots);
					m_slot_index = static_cast<std::size_t>(i);
				}
			}

			// corner pins (channel 1)
			m_slots.set_channel(is_selected ? 2 : 1);
			for (auto const& pin : slot.pin_locations) {
				m_slots.set_frame(static_cast<int>(pin));
				win.draw(m_slots);
			}

			// occupied slot item
			auto is_flying = false;
			if (m_item_flight) {
				if (m_item_flight->tag == *slot.item) { is_flying = true; }
			}
			if (slot.status == arms::SlotStatus::occupied && slot.item && !is_flying) {
				m_item_sprite.set_position(position);

				auto const id = svc.data.item_id_from_label(*slot.item);
				auto const lookup = sf::Vector2i{svc.data.item[id]["lookup"][0].as<int>(), svc.data.item[id]["lookup"][1].as<int>()} * constants::i_cell_resolution_padded;

				m_item_sprite.set_texture_rect(sf::IntRect{lookup, constants::i_resolution_vec_padded});
				win.draw(m_item_sprite);
			}
		}
	}

	// debug();

	auto const origin = m_board.get_position() - cam + sf::Vector2f{40.f, 280.f};
	auto constexpr spacing = 40.f;

	// items
	// highlight by tag so the display order can differ from the selection order.
	auto display_index = std::size_t{};
	auto const selected_tag = has_flag_set(LayoutGizmoFlags::item_select_mode) && m_item_selection < m_selectable_items.size() ? std::string_view{m_selectable_items[m_item_selection]} : std::string_view{};

	for (auto& piece : player.catalog.inventory.items_view()) {
		if (piece.item->is_invisible() || !piece.item->is_extension()) { continue; }

		auto const& tag = piece.item->get_label();
		auto const selected = !selected_tag.empty() && tag == selected_tag;
		auto const where = origin + sf::Vector2f{static_cast<float>(display_index++) * spacing, -90.f - (selected ? selected_fixture_offset_v : 0.f)};

		piece.item->render(win, m_item_sprite.get_sprite(), where);

		auto const count_offset = sf::Vector2f{0.f, 16.f};
		for (auto& display : m_number_displays) {
			if (display.matches(piece.item->get_id())) { display.render(win, where + count_offset); }
		}
	}
	if (m_item_flight) {
		auto const item_data = m_services->data.get_item_json_from_tag(m_item_flight->tag);
		auto const lookup = sf::Vector2i{item_data["lookup"][0].as<int>(), item_data["lookup"][1].as<int>()} * constants::i_cell_resolution_padded;

		m_item_sprite.set_position(m_item_flight->position);
		m_item_sprite.set_texture_rect(sf::IntRect{lookup, constants::i_resolution_vec_padded});
		win.draw(m_item_sprite);
	}

	if (m_menu) { m_menu->render(win); }

	if (m_effect) { m_effect->render(win, {}); }
}

bool LayoutGizmo::handle_inputs(input::InputSystem& controller, audio::Soundboard& soundboard) {
	if (controller.digital(input::DigitalAction::menu_close).triggered) {
		close_menu();
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
		if (has_flag_set(LayoutGizmoFlags::item_select_mode)) {
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

		if (has_flag_set(LayoutGizmoFlags::item_select_mode)) { return handle_activated_slot(soundboard); }

		auto const status = m_current_layout->get_slots()[m_slot_index].status;

		switch (status) {
		case arms::SlotStatus::unavailable: handle_unavailable_slot(soundboard); break;
		case arms::SlotStatus::available:
		case arms::SlotStatus::occupied: open_slot_menu(status); break;
		default: break;
		}

		return false;
	}

	if (controller.menu_move(input::MoveDirection::left, input::DigitalActionQueryType::repeat)) {
		if (has_flag_set(LayoutGizmoFlags::item_select_mode)) {
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
		if (has_flag_set(LayoutGizmoFlags::item_select_mode)) {
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

void LayoutGizmo::exit_item_select_mode() {
	set_flag(LayoutGizmoFlags::item_select_mode, false);
	m_selectable_items.clear();
	m_item_selection = 0;
	m_current_item.reset();
	refresh();
}

void LayoutGizmo::close_menu() { m_menu.reset(); }

void LayoutGizmo::open_slot_menu(arms::SlotStatus status) {
	if (status == arms::SlotStatus::available) { m_menu.emplace(make_menu("fixture_menu", {"add_item", "cancel"})); }
	if (status == arms::SlotStatus::occupied) { m_menu.emplace(make_menu("extension_menu", {"swap", "remove", "cancel"})); }
}

void LayoutGizmo::handle_menu_selection(audio::Soundboard& soundboard) {
	auto const status = m_current_layout->get_slots()[m_slot_index].status;
	auto const item = m_current_layout->get_slots()[m_slot_index].item;
	if (status == arms::SlotStatus::available) {
		switch (m_menu->get_selection()) {
		case 0: // add item
			enter_item_select_mode();
			soundboard.play_sound("pioneer_sync");
			close_menu();
			break;
		default: // cancel
			break;
		}
	}
	if (status == arms::SlotStatus::occupied) {
		switch (m_menu->get_selection()) {
		case 0: { // swap
			enter_item_select_mode();
			soundboard.play_sound("pioneer_sync");
			close_menu();
			break;
		}
		case 1: { // remove
			m_current_layout->remove_item(m_slot_index);
			auto const from = get_slot_position(m_slot_index, m_cam);
			if (auto const to = get_item_position(*item, m_cam)) { start_item_flight(*item, from, *to, true); }
			m_current_layout->remove_item(m_slot_index);
			m_player->catalog.inventory.recuperate_item(*item);
			refresh();
			soundboard.play_sound("item_unequip");
			close_menu();
			break;
		}
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

		if (m_selectable_items.empty()) { return false; }

		m_current_item.emplace(m_selectable_items[m_item_selection]);
	}

	auto const item_data = m_services->data.get_item_json_from_tag(*m_current_item);
	auto const item_pin = static_cast<arms::PinLocation>(item_data["pin_location"].as<int>());

	if (!m_current_layout->can_place_item(item_pin, m_slot_index)) {
		soundboard.play_sound("error");
		m_services->notifications.push_notification(*m_services, m_services->data.gui_text["notifications"]["mismatched_item_pin"].as_string());
		exit_item_select_mode();
		return false;
	}

	if (m_player->catalog.inventory.get_available(*m_current_item) > 0) {
		if (m_current_layout->place_item(*m_current_item, item_pin, m_slot_index)) {
			auto const from = get_item_position(*m_current_item, m_cam);
			auto const to = get_slot_position(m_slot_index, m_cam);

			if (from) { start_item_flight(*m_current_item, *from, to, false); }

			soundboard.play_sound("pioneer_slot");
			m_player->catalog.inventory.use_item(*m_current_item);
			exit_item_select_mode();
			return false;
		}
	} else {
		soundboard.play_sound("error");
		m_services->notifications.push_notification(*m_services, m_services->data.gui_text["notifications"]["no_available_extensions"].as_string());
		exit_item_select_mode();
		return false;
	}

	soundboard.play_sound("error");
	m_current_item.reset();
	return false;
}

void LayoutGizmo::handle_unavailable_slot(audio::Soundboard& soundboard) { soundboard.play_sound("error"); }

void LayoutGizmo::exit_menu_if_finished() {
	if (!m_menu) { return; }
	if (m_menu->was_closed() || (m_menu->was_selected() && m_menu->was_last_option())) { m_menu.reset(); }
}

void LayoutGizmo::refresh() {
	m_number_displays.clear();
	for (auto& piece : m_player->catalog.inventory.items_view()) {
		if (piece.item->is_invisible()) { continue; }
		if (!piece.item->is_extension() && !piece.item->is_special()) { continue; }
		m_number_displays.push_back(NumberDisplay(*m_services, m_player->catalog.inventory.get_available(piece.item->get_label()), piece.item->get_id()));
		if (!piece.item->is_extension()) { continue; }
	}
}

void LayoutGizmo::start_item_flight(std::string_view tag, sf::Vector2f from, sf::Vector2f to, bool removing) {
	m_item_flight.emplace(ItemFlight{.from = from, .to = to, .position = from, .tag = std::string{tag}, .elapsed = 0.f, .duration = 0.12f, .removing = removing});
}

void LayoutGizmo::update_item_flight(float dt) {
	if (!m_item_flight) { return; }

	auto& flight = *m_item_flight;
	flight.elapsed += dt;

	if (flight.elapsed >= flight.duration) {
		m_effect.emplace(entity::Effect{*m_services, "diamond_flash", m_item_flight->position});
		m_item_flight.reset();
		return;
	}
	auto const t = flight.elapsed / flight.duration;
	auto const eased = t < 0.5f ? 4.f * t * t * t : 1.f - std::pow(-2.f * t + 2.f, 3.f) / 2.f;

	flight.position = flight.from + (flight.to - flight.from) * eased;
}

MiniMenu LayoutGizmo::make_menu(std::string_view key, std::initializer_list<std::string_view> options) {
	auto& gui_text = m_services->data.gui_text;
	std::vector<std::string> labels;
	labels.reserve(options.size());

	for (auto const option : options) { labels.push_back(gui_text[key][option].as_string()); }

	return MiniMenu{*m_services, std::move(labels), m_services->window->f_center_screen(), *p_theme};
}

auto LayoutGizmo::get_item_position(std::string_view tag, sf::Vector2f cam) const -> std::optional<sf::Vector2f> {
	auto const origin = m_board.get_position() - cam + sf::Vector2f{40.f, 280.f};
	auto constexpr spacing = 40.f;
	auto display_index = std::size_t{};

	auto const selected_tag = has_flag_set(LayoutGizmoFlags::item_select_mode) && m_item_selection < m_selectable_items.size() ? std::string_view{m_selectable_items[m_item_selection]} : std::string_view{};

	for (auto const& piece : m_player->catalog.inventory.items_view()) {
		if (piece.item->is_invisible() || !piece.item->is_extension()) { continue; }

		auto const& item_tag = piece.item->get_label();
		auto const selected = !selected_tag.empty() && item_tag == selected_tag;
		auto const where = origin + sf::Vector2f{static_cast<float>(display_index++) * spacing, -90.f - (selected ? selected_fixture_offset_v : 0.f)};

		if (item_tag == tag) { return where; }
	}

	return std::nullopt;
}

auto LayoutGizmo::get_slot_position(std::size_t const index, sf::Vector2f cam) const -> sf::Vector2f {
	auto const base = m_board.get_global_center() - cam;
	auto constexpr half = 20.f;
	auto constexpr offset = half * arms::layout_size_v;

	auto const y = index % 2 == 0 ? -half : half;
	auto position = base + sf::Vector2f{static_cast<float>(index) * half * 2.f - offset, y};

	auto const& slot = m_current_layout->get_slots()[index];
	if (slot.index && has_flag_set(LayoutGizmoFlags::selected)) {
		if (m_selection.get() == static_cast<int>(*slot.index)) { position.y -= selected_fixture_offset_v; }
	}

	return position;
}

void LayoutGizmo::debug() {
	static auto sz = ImVec2{440.f, 420.f};
	ImGui::SetNextWindowSize(sz, ImGuiCond_FirstUseEver);

	if (ImGui::Begin("Layout Debug")) {
		ImGui::SeparatorText("Mode");
		ImGui::Text("Item Select Mode: %s", has_flag_set(LayoutGizmoFlags::item_select_mode) ? "ON" : "");
		if (ImGui::CollapsingHeader("Layout Gizmo", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::Text("Layout: %s", m_current_layout ? "assigned" : "none");
			ImGui::Text("Selected: %s", has_flag_set(LayoutGizmoFlags::selected) ? "yes" : "no");
			ImGui::Text("Item select mode: %s", has_flag_set(LayoutGizmoFlags::item_select_mode) ? "yes" : "no");
			ImGui::Separator();

			if (m_current_layout != nullptr) {
				auto const& slots = m_current_layout->get_slots();
				ImGui::Text("Slot count: %d", m_current_layout->get_number_of_slots());
				ImGui::Text("Selection: %d", m_selection.get());
				ImGui::Text("Slot index: %zu", m_slot_index);
				ImGui::Text("Selectable items: %zu", m_selectable_items.size());

				ImGui::Separator();

				for (auto [i, slot] : std::views::enumerate(slots)) {
					ImGui::PushID(static_cast<int>(i));

					auto const selected = slot.index && m_selection.get() == static_cast<int>(*slot.index);
					if (selected) {
						ImGui::TextColored(ImVec4{0.3f, 1.f, 0.3f, 1.f}, "Slot %zu", i);
					} else {
						ImGui::Text("Slot %zu", i);
					}

					ImGui::SameLine();
					switch (slot.status) {
					case arms::SlotStatus::unavailable: ImGui::TextDisabled("Unavailable"); break;
					case arms::SlotStatus::available: ImGui::Text("Available"); break;
					case arms::SlotStatus::occupied: ImGui::Text("Occupied"); break;
					}

					ImGui::Text("  Index: ");
					if (slot.index) {
						ImGui::SameLine();
						ImGui::Text("%zu", *slot.index);
					} else {
						ImGui::SameLine();
						ImGui::TextDisabled("none");
					}

					ImGui::Text("  Pins: ");
					ImGui::SameLine();
					if (slot.pin_locations.empty()) {
						ImGui::TextDisabled("none");
					} else {
						for (auto [pin_i, pin] : std::views::enumerate(slot.pin_locations)) {
							if (pin_i > 0) { ImGui::SameLine(); }
							ImGui::Text("%s%d", pin_i > 0 ? ", " : "", static_cast<int>(pin));
						}
					}

					ImGui::Text("  Item: ");
					ImGui::SameLine();
					if (slot.item) {
						ImGui::TextUnformatted(slot.item->c_str());
					} else {
						ImGui::TextDisabled("empty");
					}

					ImGui::Separator();
					ImGui::PopID();
				}

				if (m_menu) {
					ImGui::Text("Menu: open");
					ImGui::Text("Menu selection: %d", m_menu->get_selection());
				} else {
					ImGui::TextDisabled("Menu: closed");
				}
			}
		}

		ImGui::SeparatorText("Positions");
		ImGui::Text("Path: (%.1f, %.1f)", m_path.get_position().x, m_path.get_position().y);
		ImGui::Text("Board: (%.1f, %.1f)", m_board.get_global_center().x, m_board.get_global_center().y);
	}
	ImGui::End();
}

} // namespace fornani::gui
