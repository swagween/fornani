
#include "editor/canvas/Map.hpp"
#include <algorithm>

namespace pi {

void Map::set_labels() {
	auto ctr{0};
	for(auto& layer : layers) {
		layer.label = ctr < m_middleground ? "background " + std::to_string(ctr) : ctr == m_middleground ? "middleground / collidable" : ctr == layers.size() - 1 ? "obscuring" : "foreground " + std::to_string(ctr - m_middleground);
		++ctr;
	}
}

void Map::set_middleground(int to_middleground) {
	if (layers.empty()) { return; }
	to_middleground = std::clamp(to_middleground, 0, static_cast<int>(layers.size()) - 1);
	m_middleground = to_middleground;
}

}
