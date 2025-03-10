
#include "fornani/gui/gizmos/WardrobeGizmo.hpp"

#include "fornani/entities/player/Player.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/utils/Circuit.hpp"
#include "fornani/utils/Math.hpp"
#include "fornani/world/Map.hpp"

#include <numbers>

namespace fornani::gui {

WardrobeGizmo::WardrobeGizmo(automa::ServiceProvider& svc, world::Map& map, sf::Vector2f placement)
	: Gizmo("Wardrobe", false), m_path{svc.finder, std::filesystem::path{"/data/gui/gizmo_paths.json"}, "wardrobe", 48, util::InterpolationType::cubic}, m_nani(svc), m_core(svc.assets.get_texture("wardrobe_gizmo_core"), {139, 255}),
	  m_light(svc.assets.get_texture("red_light"), {5, 4}), m_nani_offset{38.f, 38.f}, m_pawn_offset{106.f, 332.f}, m_light_offset{12.f, 272.f}, m_scanline{sf::Sprite{svc.assets.get_texture("portrait_scanline")}},
	  m_sprite{sf::Sprite{svc.assets.get_texture("wardrobe_gizmo")}},
	  m_health_display{.hearts{sf::Sprite{svc.assets.get_texture("pioneer_hearts")}}, .sockets{sf::Sprite{svc.assets.get_texture("pioneer_heart_sockets")}}, .position{16.f, 374}} {
	m_dashboard_port = DashboardPort::wardrobe;
	m_placement = placement;
	m_wardrobe_update = true;
	m_path.set_section("start");
	m_core.set_scale(util::constants::f_scale_vec);
	m_light.set_scale(util::constants::f_scale_vec);
	m_sprite.setScale(util::constants::f_scale_vec);
	m_scanline.setScale(util::constants::f_scale_vec);
	m_health_display.hearts.setScale(util::constants::f_scale_vec);
	m_health_display.sockets.setScale(util::constants::f_scale_vec);
	m_core.push_params("idle", {0, 1, 128, 0});
	m_core.push_params("beat", {1, 9, 32, 0}, "idle");
	m_light.push_params("blink", {0, 8, 32, -1});
	m_core.set_params("idle");
	m_light.set_params("blink");
}

void WardrobeGizmo::update(automa::ServiceProvider& svc, [[maybe_unused]] player::Player& player, [[maybe_unused]] world::Map& map, sf::Vector2f position) {
	Gizmo::update(svc, player, map, position);
	if (m_state == GizmoState::selected && m_switched) {
		on_open(svc, player, map);
	} else if (m_switched) {
		on_close(svc, player, map);
	}

	if (m_outfitter) {
		// check for outfit change
		if (m_outfitter->has_changed()) { m_wardrobe_update = true; }
		m_outfitter->update(svc, player, map, m_placement + m_path.get_position());
	}
	if (svc.ticker.every_x_ticks(600)) { m_core.set_params("beat"); }
	m_path.update();
	m_nani.set_position(m_placement + m_path.get_position() + m_nani_offset);
	m_core.update(m_placement + m_path.get_position());
	m_light.update(m_placement + m_path.get_position() + m_light_offset);

	// health display
	m_health_display.socket_state = player.health.get_i_max() - 3;
	m_health_display.hearts.setTextureRect(sf::IntRect{{0, (player.health.get_i_hp() - 1) * 43}, {80, 43}});
	m_health_display.sockets.setTextureRect(sf::IntRect{{0, m_health_display.socket_state * 47}, {89, 47}});

	// gate wardrobe updates because they're expensive
	// and don't need to be called tickwise
	if (m_wardrobe_update) {
		m_nani.update(player);
		m_wardrobe_update = false;
	}
}

void WardrobeGizmo::render(automa::ServiceProvider& svc, sf::RenderWindow& win, [[maybe_unused]] player::Player& player, sf::Vector2f cam, bool foreground) {
	if (is_foreground() != foreground) { return; }
	Gizmo::render(svc, win, player, cam);

	// insertion pins
	m_sprite.setTextureRect(sf::IntRect{{330, 10}, {7, 80}});
	auto pin_offset{sf::Vector2f{260.f, 72.f}};
	m_sprite.setPosition(m_placement + m_path.get_position() + pin_offset - cam);
	win.draw(m_sprite);

	// outfitter
	if (m_outfitter) { m_outfitter->render(svc, win, player, cam, foreground); }

	// player portrait + scanline + health display
	m_nani.render(win, cam);
	static auto movement{util::Circuit{4}};
	if (svc.ticker.every_x_frames(8)) { movement.modulate(1); }
	auto movement_vec{sf::Vector2f{-2.f, -4.f + static_cast<float>(movement.get())}};
	m_scanline.setPosition(m_placement + m_path.get_position() + m_nani_offset - cam + movement_vec);
	win.draw(m_scanline);
	m_core.render(svc, win, cam);
	player.render(svc, win, cam, m_placement + m_path.get_position() + m_pawn_offset);
	m_light.render(svc, win, cam);
	auto offset{sf::Vector2f{10.f, 4.f}};
	m_health_display.hearts.setPosition(m_placement + m_path.get_position() + m_health_display.position + offset - cam);
	m_health_display.sockets.setPosition(m_placement + m_path.get_position() + m_health_display.position - cam);
	win.draw(m_health_display.sockets);
	win.draw(m_health_display.hearts);
}

bool WardrobeGizmo::handle_inputs(config::ControllerMap& controller, [[maybe_unused]] audio::Soundboard& soundboard) {
	if (is_selected()) { m_outfitter->handle_inputs(controller, soundboard); }
	return Gizmo::handle_inputs(controller, soundboard);
}

void WardrobeGizmo::on_open(automa::ServiceProvider& svc, [[maybe_unused]] player::Player& player, [[maybe_unused]] world::Map& map) {
	Gizmo::on_open(svc, player, map);
	if (is_selected()) {
		m_outfitter = std::make_unique<OutfitterGizmo>(svc, map, sf::Vector2f{288.f, 36.f});
		m_outfitter->select();
	}
}

void WardrobeGizmo::on_close(automa::ServiceProvider& svc, [[maybe_unused]] player::Player& player, [[maybe_unused]] world::Map& map) {
	Gizmo::on_close(svc, player, map);
	m_outfitter->close();
	m_outfitter->deselect();
}

} // namespace fornani::gui
