
#include "fornani/gui/gizmos/MapInfoGizmo.hpp"
#include <numbers>
#include "fornani/entities/player/Player.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/utils/Math.hpp"
#include "fornani/world/Map.hpp"

namespace fornani::gui {

MapInfoGizmo::MapInfoGizmo(automa::ServiceProvider& svc, int room_id, sf::Vector2f placement)
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
	auto data = svc.data.get_map_data_from_id(room_id);
	if (data) { m_text.biome.setString(data->get().biome_label); }
	m_text.room.setFillColor(colors::pioneer_red);
	m_text.room.setCharacterSize(16);
	if (data) { m_text.room.setString(data->get().room_label); }
	m_clip_path.set_section("open");
	current_room = room_id;
}

void MapInfoGizmo::update(automa::ServiceProvider& svc, [[maybe_unused]] player::Player& player, [[maybe_unused]] world::Map& map, sf::Vector2f position) {
	Gizmo::update(svc, player, map, position);
	m_steering.target(m_physics, m_placement, 0.003f);
	m_physics.simple_update();
	m_clip_path.update();
	if (m_clip_path.get_section() == 1 && m_clip_path.completed_step(1)) { svc.soundboard.flags.pioneer.set(audio::Pioneer::drag); }
	auto data = svc.data.get_map_data_from_id(current_room);
	if (data) { m_text.biome.setString(data->get().biome_label); }
	if (data) { m_text.room.setString(data->get().room_label); }
}

void MapInfoGizmo::render(automa::ServiceProvider& svc, sf::RenderWindow& win, [[maybe_unused]] player::Player& player, LightShader& shader, Palette& palette, sf::Vector2f cam, bool foreground) {
	if (is_foreground() != foreground) { return; }
	Gizmo::render(svc, win, player, shader, palette, cam);
	m_sprites.panel.setPosition(m_physics.position - cam);
	m_constituents.left_clip.render(win, m_sprites.clip, cam - m_physics.position - m_clip_path.get_position(), {}, shader, palette);
	m_constituents.right_clip.render(win, m_sprites.clip, cam - m_physics.position - m_clip_path.get_position() - m_clip_path.get_dimensions(), {}, shader, palette);
	win.draw(m_sprites.panel);
	m_text.biome.setPosition(m_physics.position + sf::Vector2f{40.f, 16.f} - cam);
	m_text.room.setPosition(m_physics.position + sf::Vector2f{40.f, 40.f} - cam);
	win.draw(m_text.biome);
	win.draw(m_text.room);
}

bool MapInfoGizmo::handle_inputs(input::InputSystem& controller, [[maybe_unused]] audio::Soundboard& soundboard) { return Gizmo::handle_inputs(controller, soundboard); }

} // namespace fornani::gui
