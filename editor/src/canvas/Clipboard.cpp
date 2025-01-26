//
//  Clipboard.cpp
//  Pioneer Lab
//
//  Created by Alex Frasca on 12/4/22.
//

#include "editor/canvas/Clipboard.hpp"

namespace pi {

Clipboard::Clipboard(sf::Vector2<uint32_t> dimensions) : dimensions(dimensions) {}

void Clipboard::write_to_clipboard(uint32_t value, size_t i, size_t j, size_t layer) {
	while (layer >= cell_values.size()) { cell_values.push_back(std::vector<uint32_t>{}); }
	for (auto u{0}; u < dimensions.x; ++u) {
		for (auto v{0}; v < dimensions.y; ++v) { cell_values.at(layer).push_back(0); }
	}
	auto index = i + j * static_cast<size_t>(dimensions.x);
	cell_values.at(layer).at(index) = value;
}


void Clipboard::clear_clipboard() { cell_values.clear(); }

uint32_t Clipboard::get_value_at(size_t i, size_t j) { return cell_values.back().at(i + j * dimensions.x); }

uint32_t Clipboard::get_value_at(size_t i, size_t j, size_t layer) { return cell_values.at(layer).at(i + j * dimensions.x); }

}
