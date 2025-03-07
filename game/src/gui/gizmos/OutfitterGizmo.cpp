
#include "fornani/gui/gizmos/OutfitterGizmo.hpp"

#include "fornani/entities/player/Player.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/utils/Math.hpp"
#include "fornani/world/Map.hpp"

#include <numbers>

namespace fornani::gui {

OutfitterGizmo::OutfitterGizmo(automa::ServiceProvider& svc, world::Map& map, sf::Vector2f placement)
	: Gizmo("Outfitter", false), m_sprite{sf::Sprite{svc.assets.get_texture("wardrobe_gizmo")}}, m_apparel_sprite{sf::Sprite{svc.assets.get_texture("inventory_items")}},
	  m_path{svc.finder, std::filesystem::path{"/data/gui/gizmo_paths.json"}, "wardrobe_outfitter", 108, util::InterpolationType::quadratic}, m_wires(svc.assets.get_texture("wardrobe_wires"), {88, 118}), m_max_slots{9},
	  m_selector({m_max_slots, 4}, {38.f, 50.f}), m_init{true}, m_grid_offset{144.f, 10.f}, m_row{{{76, 0}, {170, 18}}, {}} {
	m_placement = placement;
	m_sprite.setScale(util::constants::f_scale_vec);
	m_wires.set_scale(util::constants::f_scale_vec);
	m_apparel_sprite.setScale(util::constants::f_scale_vec);
	m_path.set_section("start");
	m_wires.push_params("idle", {0, 1, 128, -1});
	m_wires.push_params("plug", {1, 6, 20, 0}, "set");
	m_wires.push_params("set", {7, 1, 128, -1});
	m_wires.set_params("idle");
	auto row{0};
	for (auto& slider : m_sliders) {
		slider.body.constituent.lookup = sf::IntRect{{304, 26 * row}, {26, 26}};
		++row;
	}
	m_selector.set_lookup({{307, 104}, {20, 20}}); // selector lookup on texture atlas
}

void OutfitterGizmo::update(automa::ServiceProvider& svc, [[maybe_unused]] player::Player& player, [[maybe_unused]] world::Map& map, sf::Vector2f position) {
	Gizmo::update(svc, player, map, position);
	m_path.update();
	static util::Cooldown wire_sound{140};
	m_physics.position = m_path.get_position() + position;
	if (m_init) {
		m_selector.set_position(m_physics.position + m_placement + m_grid_offset, true);
		m_outfit = player.get_outfit();
		init_sliders();
		wire_sound.start();
		m_init = false;
	}

	if (m_changed) {
		player.set_outfit(m_outfit);
		m_outfit = player.get_outfit();
		m_changed = false;
	}

	// play the wire plug sound at the exact right moment
	wire_sound.update();
	if (wire_sound.is_almost_complete() && is_selected()) { svc.soundboard.flags.pioneer.set(audio::Pioneer::wires); }

	// set slider positions
	update_sliders(player);

	m_selector.set_position(m_physics.position + m_placement + m_grid_offset);
	m_selector.update();
	m_row.position = m_physics.position + m_placement + m_grid_offset + sf::Vector2f{0.f, m_selector.get_menu_position().y};

	auto wire_offset{sf::Vector2f{-42.f, 26.f}};
	m_wires.update(m_physics.position + m_placement + wire_offset);
	if (m_path.completed_step(1)) { m_wires.set_params("plug"); }
}

void OutfitterGizmo::render(automa::ServiceProvider& svc, sf::RenderWindow& win, [[maybe_unused]] player::Player& player, sf::Vector2f cam, bool foreground) {
	if (is_foreground() != foreground) { return; }
	Gizmo::render(svc, win, player, cam);
	m_sprite.setTextureRect(sf::IntRect{{0, 18}, {304, 116}});
	m_sprite.setPosition(m_physics.position + m_placement - cam);
	win.draw(m_sprite);
	if (is_selected()) {
		auto selection_origin{sf::Vector2f{-3.f, -3.f}};
		m_row.render(win, m_sprite, cam, selection_origin - sf::Vector2f{1.f, 1.f});

		// draw item sprites
		auto column{0.f};
		auto row{0.f};
		for (auto& item : player.catalog.inventory.apparel_view()) {
			m_apparel_sprite.setTextureRect(item->get_lookup());
			m_apparel_sprite.setOrigin({-6.f, -18.f}); // center sprite in window
			item->render(win, m_apparel_sprite, m_physics.position + m_placement + m_grid_offset + item->get_table_position().componentWiseMul(m_selector.get_spacing()) - cam);
		}

		m_selector.render(win, m_sprite, cam, selection_origin);
		for (auto& slider : m_sliders) { slider.body.constituent.render(win, m_sprite, cam, {}); }
	}
	m_wires.render(svc, win, cam);
	// debug();
}

bool OutfitterGizmo::handle_inputs(config::ControllerMap& controller, [[maybe_unused]] audio::Soundboard& soundboard) {
	if (controller.digital_action_status(config::DigitalAction::menu_up).triggered) {
		m_selector.move({0, -1});
		soundboard.flags.pioneer.set(audio::Pioneer::click);
	}
	if (controller.digital_action_status(config::DigitalAction::menu_down).triggered) {
		m_selector.move({0, 1});
		soundboard.flags.pioneer.set(audio::Pioneer::click);
	}
	if (controller.digital_action_status(config::DigitalAction::menu_left).triggered) {
		m_selector.move({-1, 0});
		soundboard.flags.pioneer.set(audio::Pioneer::click);
	}
	if (controller.digital_action_status(config::DigitalAction::menu_right).triggered) {
		m_selector.move({1, 0});
		soundboard.flags.pioneer.set(audio::Pioneer::click);
	}
	if (controller.digital_action_status(config::DigitalAction::menu_select).triggered) {
		m_sliders[m_selector.get_vertical_index()].selection = m_selector.get_horizonal_index();
		m_outfit.at(m_selector.get_vertical_index()) = m_selector.get_horizonal_index();
		m_changed = true;
		soundboard.flags.pioneer.set(audio::Pioneer::slot);
	}
	return Gizmo::handle_inputs(controller, soundboard);
}

void OutfitterGizmo::close() {
	m_path.set_section("close");
	m_init = true;
}

void OutfitterGizmo::init_sliders() {
	auto row{0.f};
	auto equipped{static_cast<float>(m_max_slots)};
	for (auto& slider : m_sliders) {
		slider.selection = equipped;
		auto current_apparel{m_outfit.at(static_cast<int>(row))};
		if (current_apparel != 0) {
			equipped = current_apparel;
			slider.selection = equipped;
		}
		slider.body.physics.position = m_physics.position + m_placement + m_grid_offset + m_selector.get_spacing().componentWiseMul(sf::Vector2f{equipped, row});
		slider.body.update();
		++row;
	}
}

void OutfitterGizmo::update_sliders(player::Player& player) {
	auto row{0.f};
	auto equipped{static_cast<float>(m_max_slots)};
	for (auto& slider : m_sliders) {
		auto current_apparel{m_outfit.at(static_cast<int>(row))};
		if (current_apparel != 0) { equipped = current_apparel; }
		auto target = m_physics.position + m_placement + m_grid_offset + m_selector.get_spacing().componentWiseMul(sf::Vector2f{static_cast<float>(slider.selection), row});
		slider.body.steering.target(slider.body.physics, target, 0.008f);
		slider.body.update();
		++row;
	}
}
void OutfitterGizmo::debug() {
	ImGui::SetNextWindowSize(ImVec2{256.f, 128.f});
	if (ImGui::Begin("Outfitter Debug")) {
		ImGui::Text("Selection: %i", m_selector.get_current_selection());
		ImGui::Text("Selector Menu Pos: %.0f", m_selector.get_menu_position().x);
		ImGui::SameLine();
		ImGui::Text(", %.0f", m_selector.get_menu_position().y);
		for (auto& i : m_outfit) { ImGui::Text("Outfit: %i", i); }
		ImGui::End();
	}
}

} // namespace fornani::gui
