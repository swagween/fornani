
#include "editor/canvas/Map.hpp"


namespace pi {

void Map::set_labels() {
	auto ctr{0};
	for (auto& layer : layers) {
		layer.label = ctr < m_middleground		 ? "background " + std::to_string(ctr)
					  : ctr == m_middleground	 ? "middleground / collidable"
					  : ctr == layers.size() - 1 && m_flags.has_obscuring_layer ? "obscuring"
					  : ctr == layers.size() - 2 && m_flags.has_reverse_obscuring_layer ? "reverse obscuring"
												 : "foreground " + std::to_string(ctr - m_middleground);
		++ctr;
	}
}

void Map::set_middleground(int to_middleground) {
	if (layers.empty()) { return; }
	to_middleground = ccm::ext::clamp(to_middleground, 0, static_cast<int>(layers.size()) - 1);
	m_middleground = to_middleground;
}

void Map::delete_layer_at(std::size_t const index) {
	if (layers.size() <= 1) { return; }
	std::erase_if(layers, [index](auto const& l) { return l.render_order == index; });
	reorder();
	if (index < m_middleground) { --m_middleground; }
}

void Map::add_layer(int at, int direction) {
	if (layers.empty()) { return; }
	auto dimensions = layers.back().dimensions;
	layers.insert(layers.begin() + at + direction, Layer(at, false, dimensions));
	if (direction == 0) { ++m_middleground; }
	reorder();
}

void Map::reorder() {
	auto ctr{0};
	for (auto& layer : layers) {
		layer.render_order = ctr;
		++ctr;
	}
}

}
