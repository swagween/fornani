
#include "fornani/gui/gizmos/WardrobeGizmo.hpp"

#include "fornani/entities/player/Player.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/utils/Circuit.hpp"
#include "fornani/utils/Math.hpp"
#include "fornani/world/Map.hpp"

#include <numbers>

namespace fornani::gui {

WardrobeGizmo::WardrobeGizmo(automa::ServiceProvider& svc, world::Map& map, sf::Vector2f placement)
	: Gizmo("Wardrobe", false), m_path{svc.finder, std::filesystem::path{"/data/gui/gizmo_paths.json"}, "wardrobe", 80, util::InterpolationType::linear}, m_nani(svc), m_core(svc.assets.get_texture("wardrobe_gizmo_core")),
	  m_nani_offset{24.f, 34.f}, m_pawn_offset{98.f, 322.f}, m_scanline{sf::Sprite{svc.assets.get_texture("portrait_scanline")}} {
	m_placement = placement;
	m_wardrobe_update = true;
	m_path.set_section("start");
	m_core.set_scale(util::constants::f_scale_vec);
	m_scanline.setScale(util::constants::f_scale_vec);
}

void WardrobeGizmo::update(automa::ServiceProvider& svc, [[maybe_unused]] player::Player& player, [[maybe_unused]] world::Map& map, sf::Vector2f position) {
	Gizmo::update(svc, player, map, position);
	m_path.update();
	m_nani.set_position(m_placement + m_path.get_position() + m_nani_offset);
	m_core.set_position(m_placement + m_path.get_position());
	// gate wardrobe updates because they're expensive
	// and don't need to be called tickwise
	if (m_wardrobe_update) {
		m_nani.update(svc, player);
		m_wardrobe_update = false;
	}
}

void WardrobeGizmo::render(automa::ServiceProvider& svc, sf::RenderWindow& win, [[maybe_unused]] player::Player& player, sf::Vector2f cam, bool foreground) {
	if (is_foreground() != foreground) { return; }
	Gizmo::render(svc, win, player, cam);
	m_nani.render(svc, win, cam);
	static auto movement{util::Circuit{4}};
	if (svc.ticker.every_x_frames(8)) { movement.modulate(1); }
	auto movement_vec{sf::Vector2f{-2.f, -4.f + static_cast<float>(movement.get())}};
	m_scanline.setPosition(m_placement + m_path.get_position() + m_nani_offset - cam + movement_vec);
	win.draw(m_scanline);
	m_core.render(svc, win, cam);
	// TODO: implement a nice way to render the player wherever and however I want without affecting the player's internal state
	player.render(svc, win, cam, m_placement + m_path.get_position() + m_pawn_offset);
}

bool WardrobeGizmo::handle_inputs(config::ControllerMap& controller, [[maybe_unused]] audio::Soundboard& soundboard) { return Gizmo::handle_inputs(controller, soundboard); }

} // namespace fornani::gui
