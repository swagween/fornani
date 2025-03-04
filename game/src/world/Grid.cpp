
#include "fornani/world/Grid.hpp"

#include <cmath>

namespace fornani::world {

Grid::Grid(sf::Vector2<std::uint32_t> d, dj::Json& source, float s) : dimensions(d), m_spacing(s) {
	auto size = static_cast<std::size_t>(dimensions.x * dimensions.y);
	cells.reserve(size);
	auto i{0};
	for (auto& cell : source.array_view()) {
		auto value = cell.as<int>();
		auto xidx = static_cast<std::uint32_t>(std::floor(i % dimensions.x));
		auto yidx = static_cast<std::uint32_t>(std::floor(i / dimensions.x));
		cells.push_back(Tile({xidx, yidx}, {xidx * m_spacing, yidx * m_spacing}, value, i, m_spacing));
		seed_vertex(i);
		++i;
	}
}

void Grid::check_neighbors(int i) {
	auto right = static_cast<std::size_t>(i + 1);
	auto left = static_cast<std::size_t>(i - 1);
	auto up = static_cast<std::size_t>(i - dimensions.x);
	auto down = static_cast<std::size_t>(i + dimensions.x);
	bool surrounded{true};
	bool exposed{};
	auto ui = static_cast<std::uint32_t>(i);
	// right neighbor
	if (i != cells.size() - 1 && i % dimensions.x != dimensions.x - 1) {
		if (!cells.at(right).is_solid()) { surrounded = false; }
		if (!cells.at(right).is_occupied()) { exposed = true; }
		if (cells.at(right).is_big_ramp() && cells.at(right).is_ground_ramp() && !cells.at(i).is_ramp()) { cells.at(i).flags.set(TileState::ramp_adjacent); }
	}
	// left neighbor
	if (i != 0 && i % dimensions.x != 0) {
		if (!cells.at(left).is_solid()) { surrounded = false; }
		if (!cells.at(left).is_occupied()) { exposed = true; }
		if (cells.at(left).is_big_ramp() && cells.at(left).is_ground_ramp() && !cells.at(i).is_ramp()) { cells.at(i).flags.set(TileState::ramp_adjacent); }
	}
	// top neighbor
	if (!(ui < dimensions.x)) {
		if (cells.at(up).is_solid()) { surrounded = false; }
		if (!cells.at(up).is_occupied()) { exposed = true; }
	}
	// bottom neighbor
	if (!(ui > cells.size() - dimensions.x - 1)) {
		if (cells.at(down).is_solid()) { surrounded = false; }
		if (!cells.at(down).is_occupied()) { exposed = true; }
	}
	cells.at(i).surrounded = surrounded;
	cells.at(i).exposed = exposed;
}

sf::Vector2<int> Grid::get_solid_neighbors(int index) {
	auto ret = sf::Vector2<int>{};
	auto right = static_cast<std::size_t>(index + 1);
	auto left = static_cast<std::size_t>(index - 1);
	auto up = static_cast<std::size_t>(index - dimensions.x);
	auto down = static_cast<std::size_t>(index + dimensions.x);
	auto ui = static_cast<std::uint32_t>(index);
	// left neighbor
	if (index != 0 && index % dimensions.x != 0) {
		if (cells.at(left).is_solid()) { ret.x = -1; }
	}
	// right neighbor
	if (index != cells.size() - 1 && index % dimensions.x != dimensions.x - 1) {
		if (cells.at(right).is_solid()) { ret.x = 1; }
	}
	// top neighbor
	if (!(ui < dimensions.x)) {
		if (cells.at(up).is_solid()) { ret.y = -1; }
	}
	// bottom neighbor
	if (!(ui > cells.size() - dimensions.x - 1)) {
		if (cells.at(down).is_solid()) { ret.y = 1; }
	}
	return ret;
}

void Grid::seed_vertex(int index) {
	auto& tile = cells.at(index);
	tile.set_type();
	// this function creates slants for appropriate tiles
	switch (tile.value) {
		// top left long ramp
	case CEIL_SLANT_INDEX:
		tile.bounding_box.vertices[2].y -= m_spacing / 4;
		tile.flags.set(TileState::big_ramp);
		break;
	case CEIL_SLANT_INDEX + 1:
		tile.bounding_box.vertices[2].y -= m_spacing / 2;
		tile.bounding_box.vertices[3].y -= m_spacing / 4;
		break;
	case CEIL_SLANT_INDEX + 2:
		tile.bounding_box.vertices[2].y -= m_spacing - m_spacing / 4;
		tile.bounding_box.vertices[3].y -= m_spacing / 2;
		break;
	case CEIL_SLANT_INDEX + 3:
		tile.bounding_box.vertices[2].y -= m_spacing - m_spacing / 4;
		tile.bounding_box.vertices[2].x -= m_spacing;
		break;
		// top right long ramp
	case CEIL_SLANT_INDEX + 7:
		tile.bounding_box.vertices[3].y -= m_spacing / 4;
		tile.flags.set(TileState::big_ramp);
		break;
	case CEIL_SLANT_INDEX + 6:
		tile.bounding_box.vertices[3].y -= m_spacing / 2;
		tile.bounding_box.vertices[2].y -= m_spacing / 4;
		break;
	case CEIL_SLANT_INDEX + 5:
		tile.bounding_box.vertices[3].y -= m_spacing - m_spacing / 4;
		tile.bounding_box.vertices[2].y -= m_spacing / 2;
		break;
	case CEIL_SLANT_INDEX + 4:
		tile.bounding_box.vertices[2].y -= m_spacing - m_spacing / 4;
		break;
		// top left short ramp 1
	case CEIL_SLANT_INDEX + 8:
		tile.bounding_box.vertices[2].y -= m_spacing / 2;
		tile.flags.set(TileState::big_ramp);
		break;
	case CEIL_SLANT_INDEX + 9:
		tile.bounding_box.vertices[2].y -= m_spacing / 2;
		tile.bounding_box.vertices[2].x -= m_spacing;
		break;
		// top right short ramp 1
	case CEIL_SLANT_INDEX + 11:
		tile.bounding_box.vertices[3].y -= m_spacing / 2;
		tile.flags.set(TileState::big_ramp);
		break;
	case CEIL_SLANT_INDEX + 10:
		tile.bounding_box.vertices[2].y -= m_spacing / 2;
		break;
		// top left short ramp 2
	case CEIL_SLANT_INDEX + 12:
		tile.bounding_box.vertices[2].y -= m_spacing / 2;
		tile.flags.set(TileState::big_ramp);
		break;
	case CEIL_SLANT_INDEX + 13:
		tile.bounding_box.vertices[2].y -= m_spacing / 2;
		tile.bounding_box.vertices[2].x -= m_spacing;
		break;
		// top right short ramp 2
	case CEIL_SLANT_INDEX + 15:
		tile.bounding_box.vertices[3].y -= m_spacing / 2;
		tile.flags.set(TileState::big_ramp);
		break;
	case CEIL_SLANT_INDEX + 14:
		tile.bounding_box.vertices[2].y -= m_spacing / 2;
		break;
		// bottom left long ramp
	case FLOOR_SLANT_INDEX:
		tile.bounding_box.vertices[1].y += m_spacing / 4;
		tile.flags.set(TileState::big_ramp);
		break;
	case FLOOR_SLANT_INDEX + 1:
		tile.bounding_box.vertices[0].y += m_spacing / 4;
		tile.bounding_box.vertices[1].y += m_spacing / 2;
		break;
	case FLOOR_SLANT_INDEX + 2:
		tile.bounding_box.vertices[0].y += m_spacing / 2;
		tile.bounding_box.vertices[1].y += m_spacing - m_spacing / 4;
		break;
	case FLOOR_SLANT_INDEX + 3:
		tile.bounding_box.vertices[0].y += m_spacing - m_spacing / 4;
		tile.bounding_box.vertices[1].y += m_spacing;
		tile.bounding_box.vertices[2].x -= m_spacing;
		break;
		// bottom right long ramp
	case FLOOR_SLANT_INDEX + 7:
		tile.bounding_box.vertices[0].y += m_spacing / 4;
		tile.flags.set(TileState::big_ramp);
		break;
	case FLOOR_SLANT_INDEX + 6:
		tile.bounding_box.vertices[1].y += m_spacing / 4;
		tile.bounding_box.vertices[0].y += m_spacing / 2;
		break;
	case FLOOR_SLANT_INDEX + 5:
		tile.bounding_box.vertices[1].y += m_spacing / 2;
		tile.bounding_box.vertices[0].y += m_spacing - m_spacing / 4;
		break;
	case FLOOR_SLANT_INDEX + 4:
		tile.bounding_box.vertices[1].y += m_spacing - m_spacing / 4;
		tile.bounding_box.vertices[0].y += m_spacing;
		break;
		// bottom left short ramp 1
	case FLOOR_SLANT_INDEX + 8:
		tile.bounding_box.vertices[1].y += m_spacing / 2;
		tile.flags.set(TileState::big_ramp);
		break;
	case FLOOR_SLANT_INDEX + 9:
		tile.bounding_box.vertices[0].y += m_spacing / 2;
		tile.bounding_box.vertices[1].y += m_spacing;
		tile.bounding_box.vertices[2].x -= m_spacing;
		break;
		// bottom right short ramp 1
	case FLOOR_SLANT_INDEX + 11:
		tile.bounding_box.vertices[0].y += m_spacing / 2;
		tile.flags.set(TileState::big_ramp);
		break;
	case FLOOR_SLANT_INDEX + 10:
		tile.bounding_box.vertices[1].y += m_spacing / 2;
		tile.bounding_box.vertices[0].y += m_spacing;
		break;
		// bottom left short ramp 2
	case FLOOR_SLANT_INDEX + 12:
		tile.bounding_box.vertices[1].y += m_spacing / 2;
		tile.flags.set(TileState::big_ramp);
		break;
	case FLOOR_SLANT_INDEX + 13:
		tile.bounding_box.vertices[0].y += m_spacing / 2;
		tile.bounding_box.vertices[1].y += m_spacing;
		tile.bounding_box.vertices[2].x -= m_spacing;
		break;
		// bottom right short ramp 2
	case FLOOR_SLANT_INDEX + 15:
		tile.bounding_box.vertices[0].y += m_spacing / 2;
		tile.flags.set(TileState::big_ramp);
		break;
	case FLOOR_SLANT_INDEX + 14:
		tile.bounding_box.vertices[1].y += m_spacing / 2;
		tile.bounding_box.vertices[0].y += m_spacing;
		break;
	case ceiling_single_ramp:
		tile.bounding_box.vertices[2].x -= m_spacing;
		tile.flags.set(TileState::big_ramp);
		break;
	case ceiling_single_ramp + 2:
		tile.bounding_box.vertices[2].x -= m_spacing;
		tile.flags.set(TileState::big_ramp);
		break;
	case ceiling_single_ramp + 1: tile.flags.set(TileState::big_ramp); break;
	case ceiling_single_ramp + 3: tile.flags.set(TileState::big_ramp); break;
	case floor_single_ramp:
		tile.bounding_box.vertices[1].y += m_spacing;
		tile.bounding_box.vertices[2].x -= m_spacing;
		tile.flags.set(TileState::big_ramp);
		break;
	case floor_single_ramp + 2:
		tile.bounding_box.vertices[1].y += m_spacing;
		tile.bounding_box.vertices[2].x -= m_spacing;
		tile.flags.set(TileState::big_ramp);
		break;
	case floor_single_ramp + 1:
		tile.bounding_box.vertices[0].y += m_spacing;
		tile.flags.set(TileState::big_ramp);
		break;
	case floor_single_ramp + 3:
		tile.bounding_box.vertices[0].y += m_spacing;
		tile.flags.set(TileState::big_ramp);
		break;
	default: break;
	}
	auto above = static_cast<int>(index - dimensions.x);
	if (above >= 0) {
		if (cells.at(static_cast<size_t>(above)).is_occupied()) { tile.flags.set(TileState::covered); }
	}
}

void Grid::destroy_cell(sf::Vector2<int> pos) {
	for (auto& cell : cells) {
		if (cell.scaled_position() == pos) { cell.value = 0; }
	}
}

void Grid::render(sf::RenderWindow& win, sf::Vector2<float> cam) {
	for (auto& cell : cells) { cell.render(win, m_drawbox, cam); }
}

void Grid::draw(sf::RenderTexture& tex) {
	tex.clear(sf::Color::Transparent);
	for (auto& cell : cells) { cell.draw(tex); }
	tex.display();
}

std::size_t Grid::get_index_at_position(sf::Vector2<float> position) const {
	auto start_index = sf::Vector2<std::size_t>(static_cast<std::size_t>((position.x / 32)), static_cast<std::size_t>((position.y / 32)));
	auto ret = static_cast<std::size_t>(dimensions.x) * start_index.y + start_index.x;
	return std::clamp(ret, std::size_t{0}, cells.size() - 1);
}

Tile& Grid::get_cell(std::size_t index) { return cells.at(index); }

} // namespace fornani::world
