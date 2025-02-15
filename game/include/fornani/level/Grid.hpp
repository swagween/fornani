
#pragma once

#include "Tile.hpp"

#include <djson/json.hpp>

#include <vector>

namespace fornani::world {

constexpr int CEIL_SLANT_INDEX = 448;
constexpr int FLOOR_SLANT_INDEX = 464;
constexpr int ceiling_single_ramp = 480;
constexpr int floor_single_ramp = 496;

class Grid {
  public:
	Grid() = default;
	Grid(sf::Vector2<uint32_t> d, dj::Json& source);

	sf::Vector2<uint32_t> dimensions{};

	void check_neighbors(int i);
	sf::Vector2<int> get_solid_neighbors(int index);
	void seed_vertex(int index);
	void destroy_cell(sf::Vector2<int> pos);
	void render(sf::RenderWindow& win, sf::Vector2<float> cam);
	void draw(sf::RenderTexture& tex);
	std::size_t get_index_at_position(sf::Vector2<float> position) const;
	Tile& get_cell(std::size_t index);

	std::vector<Tile> cells{};

  private:
	float spacing{32.f};
	sf::RectangleShape drawbox{};
};

} // namespace squid
