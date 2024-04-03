
#pragma once
#include <SFML/Graphics.hpp>

namespace util {

struct Constants {
	const sf::Vector2<int> tileset_scaled{16, 16};
	float const cell_size{32.0f};
	float const chunk_size{16.0f};
	const sf::Vector2<int> aspect_ratio{3840, 2160};
	const sf::Vector2<int> screen_dimensions{aspect_ratio.x / 4, aspect_ratio.y / 4};
};

} // namespace util
