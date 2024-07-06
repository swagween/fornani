
#include "Grid.hpp"

namespace world {

Grid::Grid(sf::Vector2<uint32_t> d) : dimensions(d) {
	cells.clear();
	for (uint32_t i{0}; i < dimensions.x * dimensions.y; i++) {
		auto xidx = static_cast<uint32_t>(std::floor(i % dimensions.x));
		auto yidx = static_cast<uint32_t>(std::floor(i / dimensions.x));
		auto xpos = xidx * spacing;
		auto ypos = yidx * spacing;
		cells.push_back(Tile({xidx, yidx}, {xpos, ypos}, 0));
		cells.back().position = sf::Vector2<float>(xpos, ypos);
		cells.back().scaled_position = sf::Vector2<int>(xidx, yidx); 
		cells.back().bounding_box.set_position(sf::Vector2<float>(xpos, ypos));
		cells.back().one_d_index = i;
	}
	seed_vertices();
}

void Grid::check_neighbors() {
	for (size_t i{0}; i < cells.size(); ++i) {
		if (cells.at(i).is_occupied()) {
			bool surrounded{true};
			auto ui = static_cast<uint32_t>(i);
			// right neighbor
			if (!(i == cells.size() - 1)) {
				if (!cells.at(i + 1).is_solid()) { surrounded = false; }
				if (cells.at(i + 1).is_big_ramp() && !cells.at(i).is_ramp()) { cells.at(i).flags.set(TileState::ramp_adjacent); }
			}
			// left neighbor
			if (!(i == 0)) {
				if (!cells.at(i - 1).is_solid()) { surrounded = false; }
				if (cells.at(i - 1).is_big_ramp() && !cells.at(i).is_ramp()) { cells.at(i).flags.set(TileState::ramp_adjacent); }
			}
			// top neighbor
			if (!(ui < dimensions.x)) {
				if (!cells.at(i - dimensions.x).is_solid()) { surrounded = false; }
			}
			// bottom neighbor
			if (!(ui > cells.size() - dimensions.x - 1)) {
				if (!cells.at(i + dimensions.x).is_solid()) { surrounded = false; }
			}
			cells.at(i).surrounded = surrounded;
		}
	}
}

void Grid::seed_vertices() {
	for (auto& tile : cells) {
		tile.set_type();
		// check vector bounds
		if (tile.bounding_box.vertices.size() < 4) { return; }
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
			tile.bounding_box.vertices[2].y -= spacing - shape::error;
			tile.bounding_box.vertices[3].y -= spacing - spacing / 4;
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
			tile.bounding_box.vertices[3].y -= spacing - shape::error;
			tile.bounding_box.vertices[2].y -= spacing - spacing / 4;
			break;
			// top left short ramp 1
		case CEIL_SLANT_INDEX + 8:
			tile.bounding_box.vertices[2].y -= spacing / 2;
			tile.flags.set(TileState::big_ramp);
			break;
		case CEIL_SLANT_INDEX + 9:
			tile.bounding_box.vertices[3].y -= spacing / 2;
			tile.bounding_box.vertices[2].y -= spacing - shape::error;
			break;
			// top right short ramp 1
		case CEIL_SLANT_INDEX + 11:
			tile.bounding_box.vertices[3].y -= spacing / 2;
			tile.flags.set(TileState::big_ramp);
			break;
		case CEIL_SLANT_INDEX + 10:
			tile.bounding_box.vertices[2].y -= spacing / 2;
			tile.bounding_box.vertices[3].y -= spacing - shape::error;
			break;
			// top left short ramp 2
		case CEIL_SLANT_INDEX + 12:
			tile.bounding_box.vertices[2].y -= spacing / 2;
			tile.flags.set(TileState::big_ramp);
			break;
		case CEIL_SLANT_INDEX + 13:
			tile.bounding_box.vertices[3].y -= spacing / 2;
			tile.bounding_box.vertices[2].y -= spacing - shape::error;
			break;
			// top right short ramp 2
		case CEIL_SLANT_INDEX + 15:
			tile.bounding_box.vertices[3].y -= spacing / 2;
			tile.flags.set(TileState::big_ramp);
			break;
		case CEIL_SLANT_INDEX + 14:
			tile.bounding_box.vertices[2].y -= spacing / 2;
			tile.bounding_box.vertices[3].y -= spacing - shape::error;
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
			tile.bounding_box.vertices[1].y += spacing - shape::error;
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
			tile.bounding_box.vertices[0].y += spacing - shape::error;
			break;
			// bottom left short ramp 1
		case FLOOR_SLANT_INDEX + 8:
			tile.bounding_box.vertices[1].y += spacing / 2;
			tile.flags.set(TileState::big_ramp);
			break;
		case FLOOR_SLANT_INDEX + 9:
			tile.bounding_box.vertices[0].y += spacing / 2;
			tile.bounding_box.vertices[1].y += spacing - shape::error;
			break;
			// bottom right short ramp 1
		case FLOOR_SLANT_INDEX + 11:
			tile.bounding_box.vertices[0].y += spacing / 2;
			tile.flags.set(TileState::big_ramp);
			break;
		case FLOOR_SLANT_INDEX + 10:
			tile.bounding_box.vertices[1].y += spacing / 2;
			tile.bounding_box.vertices[0].y += spacing - shape::error;
			break;
			// bottom left short ramp 2
		case FLOOR_SLANT_INDEX + 12:
			tile.bounding_box.vertices[1].y += spacing / 2;
			tile.flags.set(TileState::big_ramp);
			break;
		case FLOOR_SLANT_INDEX + 13:
			tile.bounding_box.vertices[0].y += spacing / 2;
			tile.bounding_box.vertices[1].y += spacing - shape::error;
			break;
			// bottom right short ramp 2
		case FLOOR_SLANT_INDEX + 15:
			tile.bounding_box.vertices[0].y += spacing / 2;
			tile.flags.set(TileState::big_ramp);
			break;
		case FLOOR_SLANT_INDEX + 14:
			tile.bounding_box.vertices[1].y += spacing / 2;
			tile.bounding_box.vertices[0].y += spacing - shape::error;
			break;
		default: break;
		}
		for (int i = 0; i < tile.bounding_box.vertices.size(); i++) {
			tile.bounding_box.edges[i].x = tile.bounding_box.vertices[(i + 1) % tile.bounding_box.vertices.size()].x - tile.bounding_box.vertices[i].x;
			tile.bounding_box.edges[i].y = tile.bounding_box.vertices[(i + 1) % tile.bounding_box.vertices.size()].y - tile.bounding_box.vertices[i].y;
			tile.bounding_box.normals[i] = tile.bounding_box.perp(tile.bounding_box.edges[i]);
		}
	}
}

void Grid::destroy_cell(sf::Vector2<int> pos) {
	for(auto& cell : cells) {
		if (cell.scaled_position == pos) { cell.value = 0; }
	}
}

} // namespace world
