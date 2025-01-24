
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

void Grid::match(Grid& other) {
	for (uint32_t i = 0; i < dimensions.y; i++) {
		for (uint32_t j = 0; j < dimensions.x; j++) {
			auto index = std::size_t{j + i * dimensions.x};
			auto other_index = std::size_t{j + i * other.dimensions.x};
			auto to_value = j >= other.dimensions.x ? 0 : (i >= other.dimensions.y ? 0 : other.cells.at(other_index).value);
			cells.at(index).value = to_value;
		}
	}
}

} // namespace pi