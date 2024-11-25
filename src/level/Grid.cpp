
#include "Grid.hpp"

namespace world {

Grid::Grid(sf::Vector2<uint32_t> d, dj::Json& source) : dimensions(d) {
	auto size = static_cast<std::size_t>(dimensions.x * dimensions.y);
	cells.reserve(size);
	auto i{0};
	for (auto& cell : source.array_view()) {
		auto value = cell.as<int>();
		auto xidx = static_cast<uint32_t>(std::floor(i % dimensions.x));
		auto yidx = static_cast<uint32_t>(std::floor(i / dimensions.x));
		cells.push_back(Tile({xidx, yidx}, {xidx * spacing, yidx * spacing}, value, i));
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
	auto ui = static_cast<uint32_t>(i);
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

void Grid::seed_vertex(int index) {
	auto& tile = cells.at(index);
	tile.set_type();
	// this function creates slants for appropriate tiles
	switch (tile.value) {
		// top left long ramp
	case CEIL_SLANT_INDEX:
		tile.bounding_box.vertices[2].y -= spacing / 4;
		tile.flags.set(TileState::big_ramp);
		break;
	case CEIL_SLANT_INDEX + 1:
		tile.bounding_box.vertices[2].y -= spacing / 2;
		tile.bounding_box.vertices[3].y -= spacing / 4;
		break;
	case CEIL_SLANT_INDEX + 2:
		tile.bounding_box.vertices[2].y -= spacing - spacing / 4;
		tile.bounding_box.vertices[3].y -= spacing / 2;
		break;
	case CEIL_SLANT_INDEX + 3:
		tile.bounding_box.vertices[2].y -= spacing - spacing / 4;
		tile.bounding_box.vertices[2].x -= spacing;
		break;
		// top right long ramp
	case CEIL_SLANT_INDEX + 7:
		tile.bounding_box.vertices[3].y -= spacing / 4;
		tile.flags.set(TileState::big_ramp);
		break;
	case CEIL_SLANT_INDEX + 6:
		tile.bounding_box.vertices[3].y -= spacing / 2;
		tile.bounding_box.vertices[2].y -= spacing / 4;
		break;
	case CEIL_SLANT_INDEX + 5:
		tile.bounding_box.vertices[3].y -= spacing - spacing / 4;
		tile.bounding_box.vertices[2].y -= spacing / 2;
		break;
	case CEIL_SLANT_INDEX + 4:
		tile.bounding_box.vertices[2].y -= spacing - spacing / 4;
		break;
		// top left short ramp 1
	case CEIL_SLANT_INDEX + 8:
		tile.bounding_box.vertices[2].y -= spacing / 2;
		tile.flags.set(TileState::big_ramp);
		break;
	case CEIL_SLANT_INDEX + 9:
		tile.bounding_box.vertices[2].y -= spacing / 2;
		tile.bounding_box.vertices[2].x -= spacing;
		break;
		// top right short ramp 1
	case CEIL_SLANT_INDEX + 11:
		tile.bounding_box.vertices[3].y -= spacing / 2;
		tile.flags.set(TileState::big_ramp);
		break;
	case CEIL_SLANT_INDEX + 10:
		tile.bounding_box.vertices[2].y -= spacing / 2;
		break;
		// top left short ramp 2
	case CEIL_SLANT_INDEX + 12:
		tile.bounding_box.vertices[2].y -= spacing / 2;
		tile.flags.set(TileState::big_ramp);
		break;
	case CEIL_SLANT_INDEX + 13:
		tile.bounding_box.vertices[2].y -= spacing / 2;
		tile.bounding_box.vertices[2].x -= spacing;
		break;
		// top right short ramp 2
	case CEIL_SLANT_INDEX + 15:
		tile.bounding_box.vertices[3].y -= spacing / 2;
		tile.flags.set(TileState::big_ramp);
		break;
	case CEIL_SLANT_INDEX + 14:
		tile.bounding_box.vertices[2].y -= spacing / 2;
		break;
		// bottom left long ramp
	case FLOOR_SLANT_INDEX:
		tile.bounding_box.vertices[1].y += spacing / 4;
		tile.flags.set(TileState::big_ramp);
		break;
	case FLOOR_SLANT_INDEX + 1:
		tile.bounding_box.vertices[0].y += spacing / 4;
		tile.bounding_box.vertices[1].y += spacing / 2;
		break;
	case FLOOR_SLANT_INDEX + 2:
		tile.bounding_box.vertices[0].y += spacing / 2;
		tile.bounding_box.vertices[1].y += spacing - spacing / 4;
		break;
	case FLOOR_SLANT_INDEX + 3:
		tile.bounding_box.vertices[0].y += spacing - spacing / 4;
		tile.bounding_box.vertices[1].y += spacing;
		tile.bounding_box.vertices[2].x -= spacing;
		break;
		// bottom right long ramp
	case FLOOR_SLANT_INDEX + 7:
		tile.bounding_box.vertices[0].y += spacing / 4;
		tile.flags.set(TileState::big_ramp);
		break;
	case FLOOR_SLANT_INDEX + 6:
		tile.bounding_box.vertices[1].y += spacing / 4;
		tile.bounding_box.vertices[0].y += spacing / 2;
		break;
	case FLOOR_SLANT_INDEX + 5:
		tile.bounding_box.vertices[1].y += spacing / 2;
		tile.bounding_box.vertices[0].y += spacing - spacing / 4;
		break;
	case FLOOR_SLANT_INDEX + 4:
		tile.bounding_box.vertices[1].y += spacing - spacing / 4;
		tile.bounding_box.vertices[0].y += spacing;
		break;
		// bottom left short ramp 1
	case FLOOR_SLANT_INDEX + 8:
		tile.bounding_box.vertices[1].y += spacing / 2;
		tile.flags.set(TileState::big_ramp);
		break;
	case FLOOR_SLANT_INDEX + 9:
		tile.bounding_box.vertices[0].y += spacing / 2;
		tile.bounding_box.vertices[1].y += spacing;
		tile.bounding_box.vertices[2].x -= spacing;
		break;
		// bottom right short ramp 1
	case FLOOR_SLANT_INDEX + 11:
		tile.bounding_box.vertices[0].y += spacing / 2;
		tile.flags.set(TileState::big_ramp);
		break;
	case FLOOR_SLANT_INDEX + 10:
		tile.bounding_box.vertices[1].y += spacing / 2;
		tile.bounding_box.vertices[0].y += spacing;
		break;
		// bottom left short ramp 2
	case FLOOR_SLANT_INDEX + 12:
		tile.bounding_box.vertices[1].y += spacing / 2;
		tile.flags.set(TileState::big_ramp);
		break;
	case FLOOR_SLANT_INDEX + 13:
		tile.bounding_box.vertices[0].y += spacing / 2;
		tile.bounding_box.vertices[1].y += spacing;
		tile.bounding_box.vertices[2].x -= spacing;
		break;
		// bottom right short ramp 2
	case FLOOR_SLANT_INDEX + 15:
		tile.bounding_box.vertices[0].y += spacing / 2;
		tile.flags.set(TileState::big_ramp);
		break;
	case FLOOR_SLANT_INDEX + 14:
		tile.bounding_box.vertices[1].y += spacing / 2;
		tile.bounding_box.vertices[0].y += spacing;
		break;
	default: break;
	}
	tile.bounding_box.set_normals();
	auto above = static_cast<int>(index - dimensions.x);
	if (above >= 0) {
		if (cells.at(static_cast<size_t>(above)).is_occupied()) { tile.flags.set(TileState::covered); }
	}
}

void Grid::destroy_cell(sf::Vector2<int> pos) {
	for(auto& cell : cells) {
		if (cell.scaled_position() == pos) { cell.value = 0; }
	}
}

void Grid::render(sf::RenderWindow& win, sf::Vector2<float> cam) {
	for (auto& cell : cells) { cell.render(win, cam, drawbox); }
}

std::size_t Grid::get_index_at_position(sf::Vector2<float> position) const {
	auto start_index = sf::Vector2<std::size_t>(static_cast<std::size_t>((position.x / 32)), static_cast<std::size_t>((position.y / 32)));
	auto ret = static_cast<std::size_t>(dimensions.x) * start_index.y + start_index.x;
	return std::clamp(ret, std::size_t{0}, cells.size() - 1);
}

Tile& Grid::get_cell(int index) { return cells.at(index); }

} // namespace world
