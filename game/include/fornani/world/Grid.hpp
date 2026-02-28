
#pragma once

#include <djson/json.hpp>
#include <fornani/utils/NeighborSet.hpp>
#include <fornani/world/Tile.hpp>
#include <vector>

namespace fornani::world {

class Grid {
  public:
	Grid() = default;
	Grid(sf::Vector2u d, dj::Json& source, float s);

	sf::Vector2u dimensions{};

	void check_neighbors(int i);
	NeighborSet get_solid_neighbors(int index);
	void seed_vertex(int index);
	void destroy_cell(sf::Vector2<int> pos);
	void render(sf::RenderWindow& win, sf::Vector2f cam);
	void draw(sf::RenderTexture& tex);
	std::size_t get_index_at_position(sf::Vector2f position) const;
	Tile& get_cell(std::size_t index);

	std::vector<Tile> cells{};

  private:
	float m_spacing{};
	sf::RectangleShape m_drawbox{};
};

} // namespace fornani::world
