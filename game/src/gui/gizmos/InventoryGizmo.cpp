
#include "fornani/gui/gizmos/InventoryGizmo.hpp"
#include <numbers>
#include "fornani/entities/player/Player.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/utils/Circuit.hpp"
#include "fornani/utils/Math.hpp"
#include "fornani/world/Map.hpp"

namespace fornani::gui {

InventoryGizmo::InventoryGizmo(automa::ServiceProvider& svc, world::Map& map, sf::Vector2f placement)
	: Gizmo("Inventory", false), m_path{svc.finder, std::filesystem::path{"/data/gui/gizmo_paths.json"}, "inventory", 128, util::InterpolationType::cubic},
	  m_lid_path{svc.finder, std::filesystem::path{"/data/gui/gizmo_paths.json"}, "inventory", 128, util::InterpolationType::cubic}, m_sprite{svc.assets.get_texture("inventory_gizmo")},
	  m_item_sprite{svc.assets.get_texture("inventory_items")}, m_selector({m_max_slots, 4}, {36.f, 36.f}), m_max_slots{12}, m_inventory_offset{48.f, 114.f}, m_orb_display(svc), m_services(&svc) {
	m_dashboard_port = DashboardPort::inventory;
	m_path.set_section("start");
	m_lid_path.set_section("start");
	m_placement = placement;
	m_sprite.setScale(constants::f_scale_vec);
	m_item_sprite.setScale(constants::f_scale_vec);
	m_selector.set_lookup({{448, 0}, {18, 18}});
	m_description = std::make_unique<DescriptionGizmo>(svc, map, m_placement, sf::IntRect{}, sf::FloatRect{{572.f, 194.f}, {240.f, 180.f}}, sf::Vector2f{});
	m_description->set_text_only(true);
}

void InventoryGizmo::update(automa::ServiceProvider& svc, [[maybe_unused]] player::Player& player, [[maybe_unused]] world::Map& map, sf::Vector2f position) {
	Gizmo::update(svc, player, map, position);
	if (m_state == GizmoState::selected && m_switched) {
		on_open(svc, player, map);
	} else if (m_switched) {
		on_close(svc, player, map);
	}

	bool found{};
	for (auto& piece : player.catalog.inventory.key_items_view()) {
		if (piece->get_table_index(m_max_slots) == m_current_item_lookup) {
			m_current_item = piece.get();
			m_current_item_id = piece->get_id();
			m_flags.set(InventoryGizmoFlags::is_item_hovered);
			found = true;
		}
	}
	if (!found) { m_flags.reset(InventoryGizmoFlags::is_item_hovered); }

	m_path.update();
	m_lid_path.update();
	m_selector.set_position(m_physics.position + m_path.get_position() + m_placement + m_inventory_offset);
	m_selector.update();

	m_orb_display.update(player.wallet.get_balance());

	if (m_description) { m_description->update(svc, player, map, m_physics.position + m_path.get_position()); }
	m_current_item_lookup = m_selector.get_current_selection(m_max_slots);

	if (m_item_menu) { m_item_menu->update(svc, {4.f, 4.f}, m_selector.get_menu_position() + sf::Vector2f{128.f, 64.f}); }
	if (!is_selected()) { m_item_menu = {}; }
}

void InventoryGizmo::render(automa::ServiceProvider& svc, sf::RenderWindow& win, [[maybe_unused]] player::Player& player, sf::Vector2f cam, bool foreground) {
	if (foreground) { // lid
		m_sprite.setTextureRect(sf::IntRect{{0, 249}, {448, 249}});
		m_sprite.setPosition(m_placement + m_lid_path.get_position() - cam);
		win.draw(m_sprite);
	} else {
		Gizmo::render(svc, win, player, cam);
		m_sprite.setTextureRect(sf::IntRect{{}, {448, 249}});
		m_sprite.setPosition(m_placement + m_path.get_position() - cam);
		win.draw(m_sprite);

		if (m_description) {
			auto title_offset = sf::Vector2f{36.f, 80.f};
			for (auto& piece : player.catalog.inventory.key_items_view()) {
				if (piece->get_table_index(m_max_slots) == m_current_item_lookup) {
					m_description->adjust_bounds(cam);
					m_description->adjust_bounds(title_offset);
					m_description->write(svc, piece->get_title(), svc.text.fonts.basic);
					m_description->render(svc, win, player, cam);
					m_description->adjust_bounds(-title_offset);
					m_description->write(svc, piece->get_description(), svc.text.fonts.basic);
					m_description->render(svc, win, player, cam);
				}
			}
			if (!is_item_hovered()) {
				m_description->adjust_bounds(cam);
				m_description->write(svc, "---", svc.text.fonts.basic);
				m_description->render(svc, win, player, cam - m_path.get_position());
			}
		}

		auto orb_offset = sf::Vector2f{208.f, 419.f};
		m_orb_display.render(win, m_placement + m_path.get_position() - cam + orb_offset);

		auto slot_dim = 36.f;
		auto origin = m_placement + m_path.get_position() - cam + m_inventory_offset;
		for (auto& item : player.catalog.inventory.key_items_view()) { item->render(win, m_item_sprite, origin + item->get_f_origin() * slot_dim); }
		if (is_selected()) { m_selector.render(win, m_sprite, cam, {}); }

		if (m_item_menu) { m_item_menu->render(win); }
	}
}

bool InventoryGizmo::handle_inputs(config::ControllerMap& controller, [[maybe_unused]] audio::Soundboard& soundboard) {
	if (is_selected()) {
		if (m_item_menu) {
			m_item_menu->handle_inputs(controller, soundboard);
			if (m_item_menu->was_selected()) { handle_menu_selection(m_item_menu->get_selection()); }
			if (m_item_menu) { // need to wrap again because it might have been closed
				if (m_item_menu->was_closed()) {
					m_item_menu = {};
					controller.flush_inputs();
				}
			}
		} else {
			if (controller.digital_action_status(config::DigitalAction::menu_up).triggered) {
				m_selector.move({0, -1});
				soundboard.flags.menu.set(audio::Menu::shift);
			}
			if (controller.digital_action_status(config::DigitalAction::menu_down).triggered) {
				m_selector.move({0, 1});
				soundboard.flags.menu.set(audio::Menu::shift);
			}
			if (controller.digital_action_status(config::DigitalAction::menu_left).triggered) {
				m_selector.move({-1, 0});
				soundboard.flags.menu.set(audio::Menu::shift);
			}
			if (controller.digital_action_status(config::DigitalAction::menu_right).triggered) {
				m_selector.move({1, 0});
				soundboard.flags.menu.set(audio::Menu::shift);
			}
			if (controller.digital_action_status(config::DigitalAction::menu_select).triggered) {
				if (is_item_hovered() && m_current_item) {
					auto list = m_current_item->generate_menu_list();
					if (list.size() > 1) {
						m_item_menu = MiniMenu(*m_services, list, m_selector.get_menu_position(), true);
					} else {
						soundboard.flags.menu.set(audio::Menu::select);
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

void InventoryGizmo::handle_menu_selection(int selection) {
	if (!m_item_menu) { return; }
	NANI_LOG_DEBUG(m_logger, "menu selected at {}", selection);
	if (m_item_menu->was_last_option()) {
		m_item_menu = {};
		NANI_LOG_DEBUG(m_logger, "menu canceled");
		return;
	}
	if (m_current_item) {
		NANI_LOG_DEBUG(m_logger, "trying item...");
		if (m_current_item->is_readable()) {
			NANI_LOG_DEBUG(m_logger, "reading item...");
			m_services->events.dispatch_event("ReadItem", m_current_item_id);
			m_item_menu = {};
		}
	}
}

} // namespace fornani::gui
