
#include "fornani/gui/gizmos/OutfitterGizmo.hpp"

#include "fornani/entities/player/Player.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/utils/Math.hpp"
#include "fornani/world/Map.hpp"

#include <numbers>

namespace fornani::gui {

OutfitterGizmo::OutfitterGizmo(automa::ServiceProvider& svc, world::Map& map, sf::Vector2f placement)
	: Gizmo("Outfitter", false), m_sprite{sf::Sprite{svc.assets.get_texture("wardrobe_gizmo")}}, m_path{svc.finder, std::filesystem::path{"/data/gui/gizmo_paths.json"}, "wardrobe_outfitter", 108, util::InterpolationType::quadratic},
	  m_wires(svc.assets.get_texture("wardrobe_wires"), {88, 118}), m_max_slots{10} {
	m_placement = placement;
	m_sprite.setScale(util::constants::f_scale_vec);
	m_wires.set_scale(util::constants::f_scale_vec);
	m_path.set_section("start");
	m_wires.push_params("idle", {0, 1, 128, -1});
	m_wires.push_params("plug", {1, 6, 20, 0}, "set");
	m_wires.push_params("set", {7, 1, 128, -1});
	m_wires.set_params("idle");
	auto row{0};
	for (auto& slider : m_sliders) {
		slider.lookup = sf::IntRect{{304, 26 * row}, {26, 26}};
		++row;
	}
}

void OutfitterGizmo::update(automa::ServiceProvider& svc, [[maybe_unused]] player::Player& player, [[maybe_unused]] world::Map& map, sf::Vector2f position) {
	Gizmo::update(svc, player, map, position);
	m_path.update();
	m_physics.position = m_path.get_position() + position;

	// set slider positions
	auto slider_offset{sf::Vector2f{144.f, 10.f}};
	auto grid_offset{sf::Vector2f{38.f, 50.f}};
	auto row{0.f};
	auto equipped{static_cast<float>(m_max_slots) - 1.f};
	for (auto& slider : m_sliders) {
		auto current_apparel{player.catalog.wardrobe.get_variant(static_cast<player::ApparelType>(row))};
		if (current_apparel != player::ClothingVariant::standard) { equipped = static_cast<int>(current_apparel); }
		slider.position = m_physics.position + m_placement + slider_offset + grid_offset.componentWiseMul(sf::Vector2f{equipped, row});
		++row;
	}
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
	for (auto& slider : m_sliders) { slider.render(win, m_sprite, cam, {}); }
	m_wires.render(svc, win, cam);
}

bool OutfitterGizmo::handle_inputs(config::ControllerMap& controller, [[maybe_unused]] audio::Soundboard& soundboard) { return Gizmo::handle_inputs(controller, soundboard); }

void OutfitterGizmo::close() { m_path.set_section("close"); }

} // namespace fornani::gui
