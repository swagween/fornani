
#pragma once

#include <SFML/Graphics.hpp>
#include <fornani/components/PhysicsComponent.hpp>
#include <fornani/components/SteeringBehavior.hpp>
#include <fornani/graphics/MapTexture.hpp>
#include <fornani/gui/DottedLine.hpp>
#include <fornani/io/Logger.hpp>
#include <fornani/world/Map.hpp>

namespace fornani::player {
class Player;
}

namespace fornani::entity {
class Portal;
}

namespace fornani::gui {

enum class MiniMapFlags { open };
enum class MapIconFlags { nani, gunsmith, save, chest, bed, door, boss, gobe, vendor, quest };
enum class ChunkType { top_left, top, top_right, bottom_left, bottom, bottom_right, left, right, inner };
enum class QuestMarkerType { main };

struct MapIcon {
	MapIconFlags type{};
	sf::Vector2f position{};
	int room_id{};
};

struct DoorConnection {
	int source{};
	int destination{};
	DottedLine line;
};

class MiniMap final : public Flaggable<MiniMapFlags> {
  public:
	explicit MiniMap(automa::ServiceProvider& svc);
	void set_textures(automa::ServiceProvider& svc);
	void set_markers(world::Map& map, player::Player& player);
	void add_quest_marker(QuestMarkerType type, int room_id);
	void bake(automa::ServiceProvider& svc, dj::Json const& in);
	void bake(automa::ServiceProvider& svc, world::Map& map, player::Player& player, int room, bool current = false, bool undiscovered = false);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, player::Player& player, sf::Vector2f cam, sf::Sprite& icon_sprite);
	void update();
	void clear_atlas();
	void move(sf::Vector2f direction);
	void zoom(float amount);
	void center();
	void set_port_position(sf::Vector2f to_position);
	void set_port_dimensions(sf::Vector2f to_dimensions);
	[[nodiscard]] auto hit_zoom_limit() const -> bool { return m_zoom_limit; }
	[[nodiscard]] auto hit_horiz_pan_limit() const -> bool { return m_pan_limit_x; }
	[[nodiscard]] auto hit_vert_pan_limit() const -> bool { return m_pan_limit_y; }
	[[nodiscard]] auto get_position() const -> sf::Vector2f { return m_physics.position; }
	[[nodiscard]] auto get_extent() const -> sf::FloatRect { return m_extent; }
	[[nodiscard]] auto get_center_position() const -> sf::Vector2f { return m_center_position; }
	[[nodiscard]] auto get_scale() const -> float { return m_scale; }
	[[nodiscard]] auto get_ratio() const -> float { return 32.f / m_scale; }
	[[nodiscard]] auto get_ratio_vec2() const -> sf::Vector2f { return sf::Vector2f{get_ratio(), get_ratio()}; }
	[[nodiscard]] auto get_currently_hovered_room() const -> int { return m_currently_hovered_room; }

  private:
	bool m_zoom_limit{};
	bool m_pan_limit_x{};
	bool m_pan_limit_y{};
	int m_currently_hovered_room{};
	int m_previously_hovered_room{};
	float m_scale{8.f};
	float m_speed{};
	float m_texture_scale{};
	Resolution m_resolution{};
	sf::FloatRect m_extent{};
	sf::Vector2f m_port_position{};
	sf::Vector2f m_port_dimensions{};
	sf::Vector2f m_center_position{};
	sf::Vector2f m_player_position{};
	sf::Vector2f m_target_position{};
	sf::View m_view{};
	components::PhysicsComponent m_physics{};
	components::SteeringBehavior m_steering{};
	MapTexture m_texture;
	std::optional<sf::Sprite> m_cursor;
	std::optional<sf::Sprite> m_map_sprite;
	sf::RectangleShape m_border{};
	std::vector<MapIcon> m_markers{};
	std::vector<DoorConnection> m_dotted_lines{};
	std::vector<std::unique_ptr<MapTexture>> m_atlas{};

	io::Logger m_logger{"MiniMap"};
};

} // namespace fornani::gui
