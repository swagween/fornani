
#include "fornani/gui/gizmos/MapInfoGizmo.hpp"

#include "fornani/entities/player/Player.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/utils/Math.hpp"
#include "fornani/world/Map.hpp"

#include <numbers>

namespace fornani::gui {

MapInfoGizmo::MapInfoGizmo(automa::ServiceProvider& svc, world::Map& map, sf::Vector2f placement)
	: Gizmo("Map Info", false), m_sprites{.panel{sf::Sprite{svc.assets.get_texture("map_gizmo")}}, .clip{sf::Sprite{svc.assets.get_texture("map_gizmo")}}}, m_text{.biome{svc.text.fonts.basic}, .room{svc.text.fonts.basic}},
	  m_constituents{.left_clip{.lookup{{89, 151}, {97, 11}}, .position{}}, .right_clip{.lookup{{89, 162}, {39, 11}}, .position{}}},
	  m_clip_path{svc.finder, std::filesystem::path{"/data/gui/gizmo_paths.json"}, "minimap_info", 86, util::InterpolationType::linear} {
	m_physics.position = sf::Vector2f{374.f, 1600.f};
	m_placement = placement;
	m_sprites.panel.setScale(constants::f_scale_vec);
	m_sprites.clip.setScale(constants::f_scale_vec);
	m_sprites.panel.setTextureRect(sf::IntRect{{0, 110}, {219, 41}});
	m_text.biome.setFillColor(colors::pioneer_red);
	m_text.biome.setCharacterSize(16);
	m_text.biome.setString(map.get_biome_string());
	m_text.room.setFillColor(colors::pioneer_red);
	m_text.room.setCharacterSize(16);
	m_text.room.setString(map.get_room_string());
	m_clip_path.set_section("open");
}

void MapInfoGizmo::update(automa::ServiceProvider& svc, [[maybe_unused]] player::Player& player, [[maybe_unused]] world::Map& map, sf::Vector2f position) {
	Gizmo::update(svc, player, map, position);
	m_steering.target(m_physics, m_placement, 0.003f);
	m_physics.simple_update();
	m_clip_path.update();
	if (m_clip_path.get_section() == 1 && m_clip_path.completed_step(1)) { svc.soundboard.flags.pioneer.set(audio::Pioneer::drag); }
}

void MapInfoGizmo::render(automa::ServiceProvider& svc, sf::RenderWindow& win, [[maybe_unused]] player::Player& player, sf::Vector2f cam, bool foreground) {
	if (is_foreground() != foreground) { return; }
	Gizmo::render(svc, win, player, cam);
	m_sprites.panel.setPosition(m_physics.position - cam);
	m_constituents.left_clip.render(win, m_sprites.clip, cam - m_physics.position - m_clip_path.get_position(), {});
	m_constituents.right_clip.render(win, m_sprites.clip, cam - m_physics.position - m_clip_path.get_position() - m_clip_path.get_dimensions(), {});
	win.draw(m_sprites.panel);
	m_text.biome.setPosition(m_physics.position + sf::Vector2f{40.f, 16.f} - cam);
	m_text.room.setPosition(m_physics.position + sf::Vector2f{40.f, 40.f} - cam);
	win.draw(m_text.biome);
	win.draw(m_text.room);
}

bool MapInfoGizmo::handle_inputs(config::ControllerMap& controller, [[maybe_unused]] audio::Soundboard& soundboard) { return Gizmo::handle_inputs(controller, soundboard); }

} // namespace fornani::gui
