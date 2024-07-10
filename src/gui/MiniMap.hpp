
#pragma once
#include <SFML/Graphics.hpp>
#include "Console.hpp"
#include "Selector.hpp"
#include "../level/Map.hpp"
#include "../graphics/MapTexture.hpp"
#include "../utils/Circuit.hpp"

namespace player {
class Player;
}

namespace entity {
class Portal;
}

namespace gui {

enum class ChunkType { top_left, top, top_right, bottom_left, bottom, bottom_right, left, right, inner };

struct Chunk {
	sf::Vector2<uint32_t> position{};
	uint64_t one_d_index{};
	sf::RectangleShape drawbox{};
	ChunkType type{};
	void generate();
};

class MiniMap {
  public:
	MiniMap() = default;
	MiniMap(automa::ServiceProvider& svc);
	void bake(automa::ServiceProvider& svc, world::Map& map, int room, bool current = false);
	void update(automa::ServiceProvider& svc, world::Map& map, player::Player& player);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam);
	void toggle_scale();
	void move(sf::Vector2<float> direction);
	[[nodiscard]] auto get_position() const -> sf::Vector2<float> { return position; }
	[[nodiscard]] auto get_scale() const -> float{ return scale; }

  private:
	float scale{8.f};
	float global_ratio{};
	float ratio{};
	float speed{};
	sf::Vector2<float> position{};
	sf::Vector2<float> previous_position{};
	sf::Vector2<float> player_position{};
	sf::View view{};
	MapTexture texture;
	sf::RenderTexture minimap_texture{};
	sf::RectangleShape player_box{};
	sf::Sprite map_sprite{};
	sf::RectangleShape background{};
	sf::RectangleShape border{};
	sf::Color background_color{};
	std::vector<Chunk> grid{};
	std::vector<std::unique_ptr<MapTexture>> atlas{};
	util::Circuit scalar{3};
};

} // namespace gui
