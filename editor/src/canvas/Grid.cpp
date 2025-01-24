//
//  Grid.cpp
//  squid
//
//  Created by Alex Frasca on 12/26/22.
//

#include "editor/canvas/Grid.hpp"

namespace pi {

Grid::Grid(sf::Vector2<uint32_t> d) : dimensions(d) {
	auto scale{32.f};
	for (uint32_t i = 0; i < dimensions.x * dimensions.y; i++) {
		sf::Vector2<uint32_t> idx{static_cast<uint32_t>(std::floor(i % dimensions.x)), static_cast<uint32_t>(std::floor(i / dimensions.x))};
		sf::Vector2<float> pos{static_cast<float>(idx.x) * scale, static_cast<float>(idx.y) * scale};
		cells.push_back(Tile(idx, pos, 0, TILE_NULL));
	}
}

void Grid::set_position(sf::Vector2<float> to_position, float scale) {
	for (uint32_t i = 0; i < dimensions.x * dimensions.y; i++) {
		sf::Vector2<uint32_t> idx{static_cast<uint32_t>(std::floor(i % dimensions.x)), static_cast<uint32_t>(std::floor(i / dimensions.x))};
		sf::Vector2<float> pos{static_cast<float>(idx.x) * scale, static_cast<float>(idx.y) * scale};
		cells.at(i).set_position(to_position + pos);
	}
}

} // namespace pi
