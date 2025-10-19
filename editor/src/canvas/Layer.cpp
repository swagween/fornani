
#include "editor/canvas/Layer.hpp"

namespace pi {

void Layer::clear() {
	for (auto& cell : grid.cells) { cell.value = 0; }
}

void Layer::erase() { grid.cells.clear(); }

void Layer::set_position(sf::Vector2f to_position, float scale) { grid.set_position(to_position, scale); }

} // namespace pi
