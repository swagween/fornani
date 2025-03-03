
#pragma once
#include <SFML/Graphics.hpp>
#include "Console.hpp"
#include "Selector.hpp"
#include "fornani/graphics/MapTexture.hpp"
#include "fornani/io/Logger.hpp"
#include "fornani/utils/Circuit.hpp"
#include "fornani/world/Map.hpp"

namespace fornani::player {
class Player;
}

namespace fornani::entity {
class Portal;
}

namespace fornani::gui {

enum class MapIconFlags { nani, gunsmith, save, chest, bed, door, boss, gobe, vendor };

struct MapIcon {
	MapIconFlags type{};
	sf::Vector2f position{};
	int room_id{};
};

enum class ChunkType : uint8_t { top_left, top, top_right, bottom_left, bottom, bottom_right, left, right, inner };

class MiniMap {
  public:
	explicit MiniMap(automa::ServiceProvider& svc);
	void bake(automa::ServiceProvider& svc, world::Map& map, player::Player& player, int room, bool current = false, bool undiscovered = false);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, player::Player& player, sf::Vector2<float> cam, sf::Sprite& icon_sprite);
	void clear_atlas();
	void move(sf::Vector2<float> direction);
	void zoom(float amount);
	void center();
	void set_port_position(sf::Vector2f to_position);
	void set_port_dimensions(sf::Vector2f to_dimensions);
	[[nodiscard]] auto get_position() const -> sf::Vector2<float> { return position; }
	[[nodiscard]] auto get_extent() const -> sf::FloatRect { return extent; }
	[[nodiscard]] auto get_center_position() const -> sf::Vector2<float> { return center_position; }
	[[nodiscard]] auto get_scale() const -> float { return scale; }
	[[nodiscard]] auto get_ratio() const -> float { return 32.f / scale; }
	[[nodiscard]] auto get_ratio_vec2() const -> sf::Vector2f { return sf::Vector2f{get_ratio(), get_ratio()}; }

  private:
	float scale{8.f};
	float speed{1.5f};
	float m_texture_scale{};
	sf::FloatRect extent{};
	sf::Vector2f m_port_position{};
	sf::Vector2f m_port_dimensions{};
	sf::Vector2<float> position{};
	sf::Vector2<float> previous_position{};
	sf::Vector2<float> center_position{};
	sf::Vector2<float> player_position{};
	sf::View view{};
	MapTexture texture;
	sf::Sprite m_cursor;
	sf::Sprite map_sprite;
	util::Circuit scalar{3};
	sf::RectangleShape border{};
	sf::Color background_color{};
	std::vector<MapIcon> m_markers{};
	sf::RenderTexture minimap_texture{};
	std::vector<std::unique_ptr<MapTexture>> atlas{};

	io::Logger m_logger{"MiniMap"};
};

} // namespace fornani::gui
