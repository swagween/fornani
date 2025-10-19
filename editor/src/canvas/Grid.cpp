
#include "editor/canvas/Grid.hpp"

#include <cmath>

namespace pi {

Grid::Grid(sf::Vector2<std::uint32_t> d) : dimensions(d) {
	auto scale{32.f};
	for (std::uint32_t i = 0; i < dimensions.x * dimensions.y; i++) {
		sf::Vector2<std::uint32_t> idx{static_cast<std::uint32_t>(std::floor(i % dimensions.x)), static_cast<std::uint32_t>(std::floor(i / dimensions.x))};
		sf::Vector2f pos{static_cast<float>(idx.x) * scale, static_cast<float>(idx.y) * scale};
		cells.push_back(Tile(idx, pos, 0, i));
	}
}

void Grid::set_position(sf::Vector2f to_position, float scale) {
	for (std::uint32_t i = 0; i < dimensions.x * dimensions.y; i++) {
		sf::Vector2<std::uint32_t> idx{static_cast<std::uint32_t>(std::floor(i % dimensions.x)), static_cast<std::uint32_t>(std::floor(i / dimensions.x))};
		sf::Vector2f pos{static_cast<float>(idx.x) * scale, static_cast<float>(idx.y) * scale};
		cells.at(i).set_position(to_position + pos);
	}
}

void Grid::match(Grid& other) {
	for (std::uint32_t i = 0; i < dimensions.y; i++) {
		for (std::uint32_t j = 0; j < dimensions.x; j++) {
			auto index = std::size_t{j + i * dimensions.x};
			auto other_index = std::size_t{j + i * other.dimensions.x};
			auto to_value = j >= other.dimensions.x ? 0 : (i >= other.dimensions.y ? 0 : other.cells.at(other_index).value);
			cells.at(index).value = to_value;
		}
	}
}

sf::Vector2<std::uint32_t> Grid::first_available_ground() const {
	for (auto const& cell : cells) {
		if (static_cast<std::size_t>(cell.one_d_index + dimensions.x) >= cells.size()) { return sf::Vector2u{}; }
		if (cell.value == 0 && cells.at(cell.one_d_index + dimensions.x).is_solid()) { return sf::Vector2u{cell.grid_position()}; }
	}
	return sf::Vector2u{};
}

} // namespace pi
