
#pragma once
#include <SFML/Graphics.hpp>
#include <ccmath/math/power/sqrt.hpp>

namespace util {

inline float magnitude(sf::Vector2<float> vec) { return ccm::sqrt((vec.x * vec.x) + (vec.y * vec.y)); }
inline sf::Vector2<float> unit(sf::Vector2<float> vec) { return vec / ccm::sqrt((vec.x * vec.x) + (vec.y * vec.y)); }

} // namespace util
