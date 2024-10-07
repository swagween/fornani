
#pragma once
#include <SFML/Graphics.hpp>
#include <ccmath/math/power/sqrt.hpp>
#include <numbers>

namespace util {

inline float magnitude(sf::Vector2<float> vec) { return ccm::sqrt((vec.x * vec.x) + (vec.y * vec.y)); }
inline sf::Vector2<float> unit(sf::Vector2<float> vec) { return vec / ccm::sqrt((vec.x * vec.x) + (vec.y * vec.y)); }
inline sf::Vector2<float> round_to_even(sf::Vector2<float> input) {
	auto ret = input;
	ret = {std::floor(ret.x / 2.f), std::floor(ret.y / 2.f)};
	auto intpos = static_cast<sf::Vector2<int>>(ret);
	ret = 2.f * static_cast<sf::Vector2<float>>(intpos);
	return ret;
}

} // namespace util
