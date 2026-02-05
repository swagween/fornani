
#include "fornani/gui/gizmos/InventoryGizmo.hpp"
#include <fornani/events/InventoryEvent.hpp>
#include <numbers>
#include "fornani/entities/player/Player.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/utils/Circuit.hpp"
#include "fornani/utils/Math.hpp"
#include "fornani/world/Map.hpp"

namespace fornani::gui {

InventoryGizmo::InventoryGizmo(automa::ServiceProvider& svc, world::Map& map, player::Player& player, sf::Vector2f placement)
	: Gizmo("Inventory", false), m_path{svc.finder, std::filesystem::path{"/data/gui/gizmo_paths.json"}, "inventory", 128, util::InterpolationType::cubic},
	  m_lid_path{svc.finder, std::filesystem::path{"/data/gui/gizmo_paths.json"}, "inventory", 128, util::InterpolationType::cubic}, m_sprite{svc.assets.get_texture("inventory_gizmo")},
	  m_item_sprite{svc.assets.get_texture("inventory_items")},
	  m_zones{InventoryZone{{9, 1}, {38.f, 36.f}, {414.f, 18.f}}, InventoryZone{{11, 4}, {36.f, 36.f}, {48.f, 114.f}}, InventoryZone{{8, 2}, {42.f, 62.f}, {124.f, 279.f}}, InventoryZone{{8, 1}, {60.f, 36.f}, {404.f, 430.f}}},
	  m_selector(std::make_unique<InventorySelector>(m_zones.at(static_cast<int>(InventoryZoneType::key)).table_dimensions, m_zones.at(static_cast<int>(InventoryZoneType::key)).cell_size)), m_orb_display(svc), m_services(&svc),
	  m_equipped_items_position{472.f, 106.f}, m_player{&player} {
	m_dashboard_port = DashboardPort::inventory;
	m_path.set_section("start");
	m_lid_path.set_section("start");
	m_placement = placement;
	m_sprite.setScale(constants::f_scale_vec);
	m_item_sprite.setScale(constants::f_scale_vec);
	m_selector->set_lookup({{448, 0}, {18, 18}});
	m_description = std::make_unique<DescriptionGizmo>(svc, map, m_placement, sf::IntRect{}, sf::FloatRect{{572.f, 194.f}, {220.f, 200.f}}, sf::Vector2f{});
	m_description->set_text_only(true);
	for (auto& piece : player.catalog.inventory.items_view()) {
		if (piece.item->is_invisible()) { continue; }
		if (!piece.item->is_unique()) { m_number_displays.push_back(NumberDisplay(svc, player.catalog.inventory.get_quantity(piece.item->get_label()), piece.item->get_id())); }
	}
}

void InventoryGizmo::update(automa::ServiceProvider& svc, [[maybe_unused]] player::Player& player, [[maybe_unused]] world::Map& map, sf::Vector2f position) {
	Gizmo::update(svc, player, map, position);
	if (m_state == GizmoState::selected && m_switched) {
		on_open(svc, player, map);
	} else if (m_switched) {
		on_close(svc, player, map);
	}

	if (is_closed() && m_exit_trigger) {
		m_path.set_section("end");
		m_lid_path.set_section("end");
		m_exit_trigger = false;
	}

	auto& current_zone = m_zones.at(m_zone_iterator.get());

	if (get_zone_type() == InventoryZoneType::key) { m_selector->set_lookup({{448, 0}, {18, 18}}); }
	if (get_zone_type() == InventoryZoneType::collectible) { m_selector->set_lookup({{448, 0}, {18, 18}}); }
	if (get_zone_type() == InventoryZoneType::gizmo) { m_selector->set_lookup({{448, 18}, {22, 22}}); }
	if (get_zone_type() == InventoryZoneType::ability) { m_selector->set_lookup({{448, 40}, {20, 20}}); }

	bool found{};
	for (auto& piece : player.catalog.inventory.items_view()) {
		if (piece.item->is_invisible()) { continue; }
		if (piece.item->get_table_index(current_zone.table_dimensions.x) == m_current_item_lookup) {
			if (zone_match(piece.item->get_type())) {
				m_current_item = piece.item->get_id();
				m_flags.set(InventoryGizmoFlags::is_item_hovered);
				found = true;
			}
		}
	}

	if (!found) { m_flags.reset(InventoryGizmoFlags::is_item_hovered); }

	m_path.update();
	m_lid_path.update();
	auto selector_offset = sf::Vector2f{};
	if (get_zone_type() == InventoryZoneType::key) { selector_offset = sf::Vector2f{2.f, 2.f}; }
	if (get_zone_type() == InventoryZoneType::collectible) { selector_offset = sf::Vector2f{2.f, 2.f}; }
	if (get_zone_type() == InventoryZoneType::gizmo) { selector_offset = sf::Vector2f{6.f, 6.f}; }
	if (get_zone_type() == InventoryZoneType::ability) { selector_offset = sf::Vector2f{4.f, 4.f}; }
	m_selector->set_position(m_physics.position + m_path.get_position() + m_placement + current_zone.render_offset - selector_offset);
	m_selector->update();

	m_orb_display.update(player.wallet.get_balance());

	if (m_description) { m_description->update(svc, player, map, m_physics.position + m_path.get_position()); }
	m_current_item_lookup = m_selector->get_current_selection(current_zone.table_dimensions.x);

	if (m_item_menu) { m_item_menu->update(svc, {4.f, 4.f}, m_selector->get_menu_position() + sf::Vector2f{128.f, 64.f}); }
	if (!is_selected()) { m_item_menu = {}; }
}

void InventoryGizmo::render(automa::ServiceProvider& svc, sf::RenderWindow& win, [[maybe_unused]] player::Player& player, LightShader& shader, Palette& palette, sf::Vector2f cam, bool foreground) {
	Gizmo::render(svc, win, player, shader, palette, cam, foreground);
	auto& current_zone = m_zones.at(m_zone_iterator.get());
	if (foreground) { // lid
		m_sprite.setTextureRect(sf::IntRect{{0, 249}, {448, 249}});
		m_sprite.setPosition(m_placement + m_lid_path.get_position() - cam);
		shader.submit(win, palette, m_sprite);
	} else {
		m_sprite.setTextureRect(sf::IntRect{{}, {448, 249}});
		m_sprite.setPosition(m_placement + m_path.get_position() - cam);
		shader.submit(win, palette, m_sprite);

		if (m_description) {
			if (m_current_item) {
				if (auto* item = m_player->catalog.inventory.find_item(*m_current_item)) {
					assert(item != nullptr); // already handled
					assert(m_player->catalog.inventory.items_view().size() > 0);
					if (zone_match(item->get_type()) && !item->is_invisible()) { write_description(*item, win, player, shader, palette, cam); }
				} else {
					m_current_item.reset();
				}
			}
		}

		// draw equipment slots
		auto num_equip_slots = player.catalog.inventory.find_item_stack("equip_slot") == nullptr ? 0 : player.catalog.inventory.find_item_stack("equip_slot")->quantity;
		auto equip_slot_offset = sf::Vector2f{466.f, 100.f};
		for (auto i = 0; i < num_equip_slots + 1; ++i) {
			m_sprite.setTextureRect(sf::IntRect{{448, 63}, {22, 22}});
			m_sprite.setPosition(m_placement + m_path.get_position() - cam + equip_slot_offset + sf::Vector2f{0.f, static_cast<float>(i) * 44.f});
			shader.submit(win, palette, m_sprite);
		}

		if (!is_item_hovered()) {
			m_description->adjust_bounds(cam);
			m_description->write(svc, "---", m_services->text.fonts.basic);
			m_description->render(svc, win, player, shader, palette, cam - m_path.get_position());
		}

		auto orb_offset = sf::Vector2f{208.f, 419.f};
		auto count_offset = sf::Vector2f{32.f, 39.f};
		m_orb_display.render(win, m_placement + m_path.get_position() - cam + orb_offset);

		for (auto& item : player.catalog.inventory.items_view()) {
			if (item.item->is_invisible()) { continue; }
			auto iterator = static_cast<std::size_t>(item.item->get_type());
			if (iterator >= m_zones.size()) { continue; }
			auto const& zone = m_zones.at(iterator);
			auto where = m_placement + m_path.get_position() - cam + zone.render_offset + item.item->get_f_origin().componentWiseMul(zone.cell_size);
			item.item->render(win, m_item_sprite, where);
			for (auto& display : m_number_displays) {
				if (display.matches(item.item->get_id())) { display.render(win, where + count_offset); }
			}
		}

		for (auto [j, ei] : std::views::enumerate(player.catalog.inventory.equipped_items_view())) {
			auto item = player.catalog.inventory.find_item(ei);
			if (item == nullptr) { continue; }
			if (item->is_invisible()) { continue; }
			auto spacing = sf::Vector2f{0.f, 44.f};
			auto where = m_placement + m_path.get_position() - cam + m_equipped_items_position + spacing * static_cast<float>(j);
			item->render(win, m_item_sprite, where);
		}

		if (is_selected()) { m_selector->render(win, m_sprite, cam, {}, shader, palette); }
		if (m_item_menu) { m_item_menu->render(win); }
	}
}

bool InventoryGizmo::handle_inputs(input::InputSystem& controller, [[maybe_unused]] audio::Soundboard& soundboard) {
	if (is_selected()) {
		if (m_item_menu) {
			m_item_menu->handle_inputs(controller, soundboard);
			if (m_item_menu->was_selected()) { handle_menu_selection(*m_player, m_item_menu->get_selection()); }
			if (m_item_menu) { // need to wrap again because it might have been closed
				if (m_item_menu->was_closed()) {
					m_item_menu = {};
					controller.flush_inputs();
				}
			}
		} else {
			m_remembered_locations.at(m_zone_iterator.get()) = m_selector->get_index();
			if (controller.digital(input::DigitalAction::menu_up).triggered) {
				m_description->flush();
				if (m_selector->move_direction({0, -1}).up()) { switch_zones(-1); }
				soundboard.flags.menu.set(audio::Menu::shift);
			}
			if (controller.digital(input::DigitalAction::menu_down).triggered) {
				m_description->flush();
				if (m_selector->move_direction({0, 1}).down()) { switch_zones(1); }
				soundboard.flags.menu.set(audio::Menu::shift);
			}
			if (controller.digital(input::DigitalAction::menu_left).triggered) {
				m_description->flush();
				if (m_selector->move_direction({-1, 0}).left()) {}
				soundboard.flags.menu.set(audio::Menu::shift);
			}
			if (controller.digital(input::DigitalAction::menu_right).triggered) {
				m_description->flush();
				if (m_selector->move_direction({1, 0}).right()) {}
				soundboard.flags.menu.set(audio::Menu::shift);
			}
			if (controller.digital(input::DigitalAction::menu_select).triggered) {
				NANI_LOG_DEBUG(m_logger, "selected");
				if (is_item_hovered() && m_current_item) {
					NANI_LOG_DEBUG(m_logger, "created minimenu");
					if (m_current_item) {
						if (auto* item = m_player->catalog.inventory.find_item(*m_current_item)) {
							auto list = item->generate_menu_list(m_services->data.gui_text["item_menu"]);
							if (list.size() > 1) {
								m_item_menu = MiniMenu(*m_services, list, m_selector->get_menu_position(), true);
							} else {
								soundboard.flags.menu.set(audio::Menu::select);
							}
						} else {
							m_current_item.reset();
						}
					}
				}
			}
		}
	}
	return Gizmo::handle_inputs(controller, soundboard);
}

void InventoryGizmo::on_open(automa::ServiceProvider& svc, [[maybe_unused]] player::Player& player, [[maybe_unused]] world::Map& map) {
	Gizmo::on_open(svc, player, map);
	if (is_selected()) {}
	m_lid_path.set_section("open");
}

void InventoryGizmo::on_close(automa::ServiceProvider& svc, [[maybe_unused]] player::Player& player, [[maybe_unused]] world::Map& map) {
	Gizmo::on_close(svc, player, map);
	m_lid_path.set_section("close");
}

void InventoryGizmo::handle_menu_selection(player::Player& player, int selection) {
	if (!m_item_menu) { return; }
	NANI_LOG_DEBUG(m_logger, "menu selected at {}", selection);
	if (m_item_menu->was_last_option()) {
		m_item_menu = {};
		return;
	}
	if (m_current_item) {
		if (auto* item = m_player->catalog.inventory.find_item(*m_current_item)) {
			if (m_item_menu->get_option() == m_services->data.gui_text["item_menu"]["read"].as_string()) {
				if (item->is_readable()) {
					m_services->events.read_item_by_id_event.dispatch(*m_current_item);
					m_item_menu = {};
				}
			} else if (m_item_menu->get_option() == m_services->data.gui_text["item_menu"]["equip"].as_string() || m_item_menu->get_option() == m_services->data.gui_text["item_menu"]["unequip"].as_string()) {
				if (item->is_equippable()) {
					NANI_LOG_DEBUG(m_logger, "Equipping Item {}", *m_current_item);
					m_services->events.equip_item_by_id_event.dispatch(*m_services, *m_current_item);
					m_item_menu = {};
				}
			}
		} else {
			m_current_item.reset();
		}
	}
}

void InventoryGizmo::switch_zones(int modulation) {
	m_zone_iterator.modulate(modulation);
	auto position = m_selector->get_position();
	auto& current_zone = m_zones.at(m_zone_iterator.get());
	m_selector = std::make_unique<InventorySelector>(current_zone.table_dimensions, current_zone.cell_size);
	m_selector->set_position(position, true);
	m_selector->set_lookup({{448, 0}, {18, 18}});
	m_selector->set_selection(m_remembered_locations.at(m_zone_iterator.get()));
}

void InventoryGizmo::write_description(item::Item& piece, sf::RenderWindow& win, player::Player& player, LightShader& shader, Palette& palette, sf::Vector2f cam) {
	auto title_offset = sf::Vector2f{36.f, 80.f};
	auto& current_zone = m_zones.at(m_zone_iterator.get());
	if (piece.get_table_index(current_zone.table_dimensions.x) == m_current_item_lookup) {
		m_description->adjust_bounds(cam);
		m_description->adjust_bounds(title_offset);
		m_description->write(*m_services, piece.get_title(), m_services->text.fonts.basic);
		m_description->render(*m_services, win, player, shader, palette, cam);
		m_description->adjust_bounds(-title_offset);
		m_description->write(*m_services, piece.get_description(), m_services->text.fonts.basic);
		m_description->render(*m_services, win, player, shader, palette, cam);
	}
}

} // namespace fornani::gui
