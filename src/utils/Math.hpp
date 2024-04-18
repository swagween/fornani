
#pragma once
#include <sfml/Graphics.hpp>

namespace util {

	inline float magnitude(sf::Vector2<float> vec) { return std::sqrt((vec.x * vec.x) + (vec.y * vec.y)); }

} // namespace util
