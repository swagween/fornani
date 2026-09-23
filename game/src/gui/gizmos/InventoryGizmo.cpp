
#include "fornani/gui/gizmos/InventoryGizmo.hpp"
#include <fornani/core/ItemConstants.hpp>
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
	  m_zones{InventoryZone{item::get_item_table_dimensions(item::ItemType::ability), {38.f, 36.f}, {414.f, 18.f}},		 InventoryZone{{int(player.catalog.inventory.get_number_of_items(item::ItemType::key)), 1}, {6.f, 36.f}, {118.f, 30.f}},
			  InventoryZone{item::get_item_table_dimensions(item::ItemType::unique), {36.f, 36.f}, {50.f, 80.f}},		 InventoryZone{item::get_item_table_dimensions(item::ItemType::equippable), {36.f, 36.f}, {50.f, 200.f}},
			  InventoryZone{item::get_item_table_dimensions(item::ItemType::collectible), {38.f, 62.f}, {124.f, 280.f}}, InventoryZone{item::get_item_table_dimensions(item::ItemType::useable), {36.f, 62.f}, {20.f, 450.f}},
			  InventoryZone{item::get_item_table_dimensions(item::ItemType::gizmo), {60.f, 36.f}, {404.f, 430.f}}},
	  m_selector(std::make_unique<InventorySelector>(m_zones.at(InventoryZoneType::key).table_dimensions, m_zones.at(InventoryZoneType::key).cell_size)), m_orb_display(svc), m_services(&svc), m_equipped_items_position{506.f, 104.f},
	  m_menu_offset{96.f, -16.f}, m_player{&player}, m_horizontal_toggle{64} {
	m_zones.set_location(InventoryZoneType::key);
	p_theme.emplace(svc.data.menu_themes["mini_white"]);
	m_dashboard_port = DashboardPort::inventory;
	m_path.set_section("start");
	m_lid_path.set_section("start");
	m_placement = placement;
	m_sprite.setScale(constants::f_scale_vec);
	m_item_sprite.setScale(constants::f_scale_vec);
	m_selector->set_lookup({{448, 0}, {18, 18}});
	m_description = std::make_unique<DescriptionGizmo>(svc, map, m_placement, sf::IntRect{}, sf::FloatRect{{600.f, 182.f}, {180.f, 120.f}}, sf::Vector2f{});
	m_description->set_text_only(true);
	auto num_keys = 0;
	for (auto& piece : player.catalog.inventory.items_view()) {
		if (piece.item->is_invisible()) { continue; }
		if (piece.item->is_key()) {
			piece.item->set_table_origin({num_keys, 0});
			++num_keys;
		}
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

	auto& current_zone = m_zones.current();

	if (get_zone_type() == InventoryZoneType::key) { m_selector->set_lookup({{448, 164}, {18, 22}}); }
	if (get_zone_type() == InventoryZoneType::unique) { m_selector->set_lookup({{448, 0}, {18, 18}}); }
	if (get_zone_type() == InventoryZoneType::equippable) { m_selector->set_lookup({{448, 0}, {18, 18}}); }
	if (get_zone_type() == InventoryZoneType::collectible) { m_selector->set_lookup({{448, 0}, {18, 18}}); }
	if (get_zone_type() == InventoryZoneType::useable) { m_selector->set_lookup({{448, 0}, {18, 18}}); }
	if (get_zone_type() == InventoryZoneType::gizmo) { m_selector->set_lookup({{448, 18}, {22, 22}}); }
	if (get_zone_type() == InventoryZoneType::ability) { m_selector->set_lookup({{448, 40}, {20, 20}}); }

	bool found{};
	int num_keys = 0;
	for (auto& piece : player.catalog.inventory.items_view()) {
		if (piece.item->is_invisible()) { continue; }
		auto const lookup = piece.item->get_table_index(current_zone.table_dimensions.x);
		if (lookup == m_current_item_lookup) {
			if (zone_match(piece.item->get_type())) {
				m_current_item = piece.item->get_id();
				m_flags.set(InventoryGizmoFlags::is_item_hovered);
				found = true;
			}
		}
		if (piece.item->is_key()) { ++num_keys; }
	}

	if (!found) { m_flags.reset(InventoryGizmoFlags::is_item_hovered); }

	if (m_horizontal_toggle.is_complete()) {
		m_flags.reset(InventoryGizmoFlags::moved_left);
		m_flags.reset(InventoryGizmoFlags::moved_right);
	}
	m_horizontal_toggle.update();
	m_path.update();
	m_lid_path.update();
	auto selector_offset = sf::Vector2f{};
	if (get_zone_type() == InventoryZoneType::key) { selector_offset = sf::Vector2f{2.f, 8.f}; }
	if (get_zone_type() == InventoryZoneType::unique) { selector_offset = sf::Vector2f{2.f, 2.f}; }
	if (get_zone_type() == InventoryZoneType::equippable) { selector_offset = sf::Vector2f{2.f, 2.f}; }
	if (get_zone_type() == InventoryZoneType::collectible) { selector_offset = sf::Vector2f{2.f, 2.f}; }
	if (get_zone_type() == InventoryZoneType::useable) { selector_offset = sf::Vector2f{2.f, 2.f}; }
	if (get_zone_type() == InventoryZoneType::gizmo) { selector_offset = sf::Vector2f{6.f, 6.f}; }
	if (get_zone_type() == InventoryZoneType::ability) { selector_offset = sf::Vector2f{4.f, 4.f}; }
	m_selector->set_position(m_physics.position + m_path.get_position() + m_placement + current_zone.render_offset - selector_offset);
	m_selector->update();

	m_orb_display.update(player.wallet.get_balance(), svc.ticker.dt.count());

	if (m_description) { m_description->update(svc, player, map, m_physics.position + m_path.get_position()); }
	m_current_item_lookup = m_selector->get_current_selection(current_zone.table_dimensions.x);

	if (m_item_menu) { m_item_menu->update(svc, m_selector->get_position() + m_menu_offset); }
	if (!is_selected()) { m_item_menu.reset(); }
}

void InventoryGizmo::render(automa::ServiceProvider& svc, sf::RenderWindow& win, [[maybe_unused]] player::Player& player, LightShader& shader, Palette& palette, sf::Vector2f cam, bool foreground) {
	Gizmo::render(svc, win, player, shader, palette, cam, foreground);
	auto& current_zone = m_zones.current();
	if (foreground) { // lid
		m_sprite.setTextureRect(sf::IntRect{{0, 249}, {448, 249}});
		m_sprite.setPosition(get_placement() + m_lid_path.get_position() - cam);
		shader.submit(win, palette, m_sprite);
	} else {
		m_sprite.setTextureRect(sf::IntRect{{}, {448, 249}});
		m_sprite.setPosition(get_placement() + m_path.get_position() - cam);
		shader.submit(win, palette, m_sprite);

		if (m_description && is_selected()) {
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
		auto equip_slot_offset = sf::Vector2f{502.f, 100.f};
		for (auto i = 0; i < num_equip_slots + 1; ++i) {
			m_sprite.setTextureRect(sf::IntRect{{448, 63}, {22, 22}});
			m_sprite.setPosition(get_placement() + m_path.get_position() - cam + equip_slot_offset + sf::Vector2f{0.f, static_cast<float>(i) * 44.f});
			shader.submit(win, palette, m_sprite);
		}

		if (!is_item_hovered()) {
			m_description->adjust_bounds(cam);
			m_description->write(svc, "---", m_services->text.fonts.basic);
			m_description->render(svc, win, player, shader, palette, cam - m_path.get_position());
		}

		auto orb_offset = sf::Vector2f{208.f, 419.f};
		auto count_offset = sf::Vector2f{32.f, 41.f};
		m_orb_display.render(win, get_placement() + m_path.get_position() - cam + orb_offset);

		m_flags.set(InventoryGizmoFlags::no_useable_items);
		int num_keys = 0;
		for (auto& item : player.catalog.inventory.items_view()) {
			if (item.item->is_invisible()) { continue; }
			auto zone_type = static_cast<InventoryZoneType>(item.item->get_type());
			if (!m_zones.contains(zone_type)) { continue; }
			if (zone_type == InventoryZoneType::useable) {
				m_flags.reset(InventoryGizmoFlags::no_useable_items);
				m_sprite.setTextureRect(sf::IntRect{{0, 499}, {89, 39}});
				m_sprite.setPosition(get_placement() + m_path.get_position() - cam + sf::Vector2f{8.f, 424.f});
				shader.submit(win, palette, m_sprite);
			}
			auto const& zone = m_zones.at(zone_type);
			auto where = get_placement() + m_path.get_position() - cam + zone.render_offset + item.item->get_f_origin().componentWiseMul(zone.cell_size) - sf::Vector2f{2.f, 2.f};
			if (m_zones.get_zone() == InventoryZoneType::key && m_selector && item.item->is_key()) {
				auto key_where = where;
				auto const adjustment = 30.f;
				auto offset = m_horizontal_toggle.get_inverse_quadratic_normalized() * adjustment;
				auto const diff = std::abs(m_selector->get_current_selection() - num_keys);
				if (diff > 1) { offset = adjustment; }
				if (num_keys < m_selector->get_current_selection()) {
					if (m_flags.test(InventoryGizmoFlags::moved_left)) { offset = adjustment; }
					key_where.x -= offset;
				}
				if (num_keys > m_selector->get_current_selection()) {
					if (m_flags.test(InventoryGizmoFlags::moved_right)) { offset = adjustment; }
					key_where.x += offset;
				}
				if (num_keys == m_selector->get_current_selection()) { key_where.y -= 2.f; }
				item.item->render(win, m_item_sprite, key_where);
			} else {
				item.item->render(win, m_item_sprite, where);
			}
			for (auto& display : m_number_displays) {
				if (display.matches(item.item->get_id())) { display.render(win, where + count_offset); }
			}
			if (item.item->is_key()) { ++num_keys; }
		}

		for (auto [j, ei] : std::views::enumerate(player.catalog.inventory.equipped_items_view())) {
			auto item = player.catalog.inventory.find_item(ei);
			if (item == nullptr) { continue; }
			if (item->is_invisible()) { continue; }
			auto spacing = sf::Vector2f{0.f, 44.f};
			auto where = get_placement() + m_path.get_position() - cam + m_equipped_items_position + spacing * static_cast<float>(j);
			item->render(win, m_item_sprite, where);
		}

		// heart shards
		auto cridium_count = player.get_item_count("cridium_shard") % 4;
		if (cridium_count != 0) {
			m_sprite.setTextureRect(sf::IntRect{{451, 85 + 19 * (cridium_count - 1)}, {19, 19}});
			m_sprite.setPosition(get_placement() + m_path.get_position() - cam + sf::Vector2f{66.f, 302.f});
			win.draw(m_sprite);
		}

		if (is_selected()) { m_selector->render(win, m_sprite, cam, {}, shader, palette); }
		if (m_item_menu) { m_item_menu->render(win, cam); }
	}
}

bool InventoryGizmo::handle_inputs(input::InputSystem& controller, [[maybe_unused]] audio::Soundboard& soundboard) {
	if (is_selected()) {
		if (m_item_menu) {
			m_item_menu->handle_inputs(controller, soundboard);
			if (m_item_menu->was_selected()) { handle_menu_selection(*m_player, m_item_menu->get_selection()); }
			if (m_item_menu) { // need to wrap again because it might have been closed
				if (m_item_menu->was_closed()) {
					m_item_menu.reset();
					controller.cancel_input(input::DigitalAction::menu_back);
					controller.cancel_input(input::DigitalAction::menu_close);
				}
			}
		} else {
			m_zones.set_current_location(m_selector->get_index());
			if (controller.menu_move(input::MoveDirection::up)) {
				m_flags.set(InventoryGizmoFlags::switched);
				if (m_selector->move_direction({0, -1}).up()) {
					switch_zones(-1);
					move_horizontal(soundboard);
				} else {
					soundboard.play_sound("menu_shift");
				}
			}
			if (controller.menu_move(input::MoveDirection::down)) {
				m_flags.set(InventoryGizmoFlags::switched);
				if (m_selector->move_direction({0, 1}).down()) {
					switch_zones(1);
					move_horizontal(soundboard);
				} else {
					soundboard.play_sound("menu_shift");
				}
			}
			if (controller.menu_move(input::MoveDirection::left)) {
				m_flags.set(InventoryGizmoFlags::switched);
				if (m_selector->move_direction({-1, 0}).left()) {}
				move_horizontal(soundboard);
				m_flags.set(InventoryGizmoFlags::moved_left);
			}
			if (controller.menu_move(input::MoveDirection::right)) {
				m_flags.set(InventoryGizmoFlags::switched);
				if (m_selector->move_direction({1, 0}).right()) {}
				move_horizontal(soundboard);
				m_flags.set(InventoryGizmoFlags::moved_right);
			}
			if (controller.digital(input::DigitalAction::menu_select).triggered) {
				if (is_item_hovered() && m_current_item) {
					if (m_current_item) {
						if (auto* item = m_player->catalog.inventory.find_item(*m_current_item)) {
							auto list = item->generate_menu_list(m_services->data.gui_text["item_menu"]);
							if (list.size() > 1 && p_theme) {
								m_item_menu.emplace(MiniMenu(*m_services, list, m_selector->get_position() + m_menu_offset, p_theme.value()));
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

void InventoryGizmo::move_horizontal(audio::Soundboard& soundboard) {
	auto tag = random::random_element(std::vector<std::string>{"key_menu_shift_1", "key_menu_shift_2", "key_menu_shift_3"});
	m_zones.get_zone() == InventoryZoneType::key ? soundboard.play_sound(tag) : soundboard.play_sound("menu_shift");
	m_horizontal_toggle.start();
}

void InventoryGizmo::handle_menu_selection(player::Player& player, int selection) {
	if (!m_item_menu) { return; }
	NANI_LOG_DEBUG(m_logger, "menu selected at {}", selection);
	if (m_item_menu->was_last_option()) {
		m_item_menu.reset();
		return;
	}
	if (m_current_item) {
		if (auto* item = m_player->catalog.inventory.find_item(*m_current_item)) {
			if (m_item_menu->get_option() == m_services->data.gui_text["item_menu"]["read"].as_string()) {
				if (item->is_readable()) {
					m_services->events.read_item_by_id_event.dispatch(*m_current_item);
					m_item_menu.reset();
				}
			} else if (m_item_menu->get_option() == m_services->data.gui_text["item_menu"]["equip"].as_string() || m_item_menu->get_option() == m_services->data.gui_text["item_menu"]["unequip"].as_string()) {
				if (item->is_equippable()) {
					NANI_LOG_DEBUG(m_logger, "Equipping Item {}", *m_current_item);
					m_services->events.equip_item_by_id_event.dispatch(*m_services, *m_current_item);
					m_item_menu.reset();
				}
			} else if (m_item_menu->get_option() == m_services->data.gui_text["item_menu"]["use"].as_string()) {
				if (item->is_useable()) {
					NANI_LOG_DEBUG(m_logger, "Using Item {}", *m_current_item);
					m_services->events.use_item_by_id_event.dispatch(*m_services, *m_current_item);
					m_item_menu.reset();
				}
			}
		} else {
			m_current_item.reset();
		}
	}
}

void InventoryGizmo::switch_zones(int modulation) {
	m_zones.modulate(modulation);
	if (m_zones.get_zone() == InventoryZoneType::useable && m_flags.test(InventoryGizmoFlags::no_useable_items)) { m_zones.modulate(modulation); }
	auto position = m_selector->get_position();
	auto& current_zone = m_zones.current();
	m_selector = std::make_unique<InventorySelector>(current_zone.table_dimensions, current_zone.cell_size);
	m_selector->set_position(position, true);
	m_selector->set_lookup({{448, 0}, {18, 18}});
	m_selector->set_selection(m_zones.get_current_location());
}

void InventoryGizmo::write_description(item::Item& piece, sf::RenderWindow& win, player::Player& player, LightShader& shader, Palette& palette, sf::Vector2f cam) {
	auto const title_offset = sf::Vector2f{36.f, 82.f};
	auto const title_size = sf::Vector2f{40.f, 0.f};
	if (piece.get_table_index(m_zones.current().table_dimensions.x) == m_current_item_lookup) {
		m_description->adjust_bounds(cam);
		m_description->adjust_bounds(title_offset);
		m_description->adjust_size(title_size);
		m_description->write(*m_services, piece.get_title(), m_services->text.fonts.basic);
		m_description->render(*m_services, win, player, shader, palette, cam);

		m_description->adjust_bounds(-title_offset);
		m_description->adjust_size(-title_size);
		m_description->write(*m_services, piece.get_description(), m_services->text.fonts.basic);
		m_description->render(*m_services, win, player, shader, palette, cam);

		m_flags.reset(InventoryGizmoFlags::switched);
	}
}

} // namespace fornani::gui
