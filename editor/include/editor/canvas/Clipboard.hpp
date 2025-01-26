//
//  Clipboard.hpp
//  Pioneer Lab
//
//  Created by Alex Frasca on 12/4/22.
//

#pragma once

#include "Tile.hpp"
#include <vector>
#include <stdio.h>
#include <cassert>

namespace pi {

class Clipboard {

  public:
	Clipboard(sf::Vector2<uint32_t> dimensions);
	void write_to_clipboard(uint32_t value, size_t i, size_t j, size_t layer);
	void clear_clipboard();
	uint32_t get_value_at(size_t i, size_t j);
	uint32_t get_value_at(size_t i, size_t j, size_t layer);
	[[nodiscard]] auto empty() const -> bool { return cell_values.empty(); }
	[[nodiscard]] auto real_dimensions() const -> sf::Vector2<float> { return {static_cast<float>(dimensions.x), static_cast<float>(dimensions.y)}; }
	[[nodiscard]] auto scaled_dimensions() const -> sf::Vector2<uint32_t> { return dimensions; }

  private:
	sf::Vector2<uint32_t> dimensions{};
	std::vector<std::vector<uint32_t>> cell_values{};
};

} // namespace pi
