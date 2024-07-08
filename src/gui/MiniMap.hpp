
#pragma once
#include <SFML/Graphics.hpp>
#include "Console.hpp"
#include "Selector.hpp"

namespace player {
class Player;
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
	void update(automa::ServiceProvider& svc, world::Map& map, player::Player& player);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam);
	void toggle_scale();

  private:
	float scale{8.f};
	sf::View view{};
	sf::RenderTexture map_texture{};
	sf::RectangleShape background{};
	sf::RectangleShape tile_box{};
	sf::RectangleShape plat_box{};
	sf::RectangleShape player_box{};
	sf::Sprite map_sprite{};
	sf::Color background_color{};
	sf::Color tile_color{};
	std::vector<Chunk> grid{};
};

} // namespace gui
