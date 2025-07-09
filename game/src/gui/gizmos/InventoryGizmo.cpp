#include "fornani/gui/gizmos/InventoryGizmo.hpp"

#include "fornani/entities/player/Player.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/utils/Circuit.hpp"
#include "fornani/utils/Math.hpp"
#include "fornani/world/Map.hpp"

#include <numbers>

namespace fornani::gui {

InventoryGizmo::InventoryGizmo(automa::ServiceProvider& svc, world::Map& map, sf::Vector2f placement)
	: Gizmo("Inventory", false), m_path{svc.finder, std::filesystem::path{"/data/gui/gizmo_paths.json"}, "inventory", 128, util::InterpolationType::quadratic},
	  m_lid_path{svc.finder, std::filesystem::path{"/data/gui/gizmo_paths.json"}, "inventory", 128, util::InterpolationType::quadratic}, m_sprite{svc.assets.get_texture("inventory_gizmo")},
	  m_item_sprite{svc.assets.get_texture("inventory_items")}, m_selector({m_max_slots, 4}, {36.f, 36.f}), m_max_slots{12} {
	m_dashboard_port = DashboardPort::inventory;
	m_path.set_section("start");
	m_lid_path.set_section("start");
	m_placement = placement;
	m_sprite.setScale(constants::f_scale_vec);
	m_item_sprite.setScale(constants::f_scale_vec);
	m_selector.set_lookup({{448, 0}, {18, 18}});
	m_description = std::make_unique<DescriptionGizmo>(svc, map, m_placement, sf::IntRect{}, sf::FloatRect{{572.f, 194.f}, {212.f, 180.f}});
	m_description->set_text_only(true);
}

void InventoryGizmo::update(automa::ServiceProvider& svc, [[maybe_unused]] player::Player& player, [[maybe_unused]] world::Map& map, sf::Vector2f position) {
	Gizmo::update(svc, player, map, position);
	if (m_state == GizmoState::selected && m_switched) {
		on_open(svc, player, map);
	} else if (m_switched) {
		on_close(svc, player, map);
	}
	m_path.update();
	m_lid_path.update();
	m_selector.set_position(m_physics.position + m_placement + m_inventory_offset);
	m_selector.update();

	if (m_description) {
		m_description->update(svc, player, map, m_physics.position);
		bool found{};
		for (auto& piece : player.catalog.inventory.key_items_view()) {
			if (piece->get_id() == m_current_item_id) {
				auto const& this_item = piece;
				m_description->write(svc, this_item->get_title() + ": " + this_item->get_description());
				found = true;
			}
		}
		if (!found) { m_description->write(svc, "---"); }
	}
	m_current_item_id = m_selector.get_current_selection(m_max_slots) + 1;
}

void InventoryGizmo::render(automa::ServiceProvider& svc, sf::RenderWindow& win, [[maybe_unused]] player::Player& player, sf::Vector2f cam, bool foreground) {
	if (is_foreground() != foreground) { return; }
	Gizmo::render(svc, win, player, cam);
	m_sprite.setTextureRect(sf::IntRect{{}, {448, 249}});
	m_sprite.setPosition(m_placement + m_path.get_position() - cam);
	win.draw(m_sprite);
	m_sprite.setTextureRect(sf::IntRect{{0, 249}, {448, 249}});
	m_sprite.setPosition(m_placement + m_lid_path.get_position() - cam);
	win.draw(m_sprite);

	if (m_description) { m_description->render(svc, win, player, cam); }

	auto origin = m_placement + m_path.get_position() - cam + m_inventory_offset;
	auto slot_dim = 36.f;
	auto row{0};
	auto col{0};
	for (auto& item : player.catalog.inventory.key_items_view()) {
		auto pos = origin + sf::Vector2f{static_cast<float>(col), static_cast<float>(row)} * slot_dim;
		item->render(win, m_item_sprite, pos);
		++col;
		if (col >= 12) {
			col = 0;
			++row;
		}
	}
	if (is_selected()) { m_selector.render(win, m_sprite, cam, {}); }
}

bool InventoryGizmo::handle_inputs(config::ControllerMap& controller, [[maybe_unused]] audio::Soundboard& soundboard) {
	if (is_selected()) {
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
		if (controller.digital_action_status(config::DigitalAction::menu_select).triggered) { /* open minimenu */
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

} // namespace fornani::gui
