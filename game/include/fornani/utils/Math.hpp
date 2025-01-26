
#pragma once
#include <SFML/Graphics.hpp>
#include <ccmath/math/power/sqrt.hpp>
#include <ccmath/math/power/pow.hpp>
#include <ccmath/ext/cubic.hpp>
#include <ccmath/ext/smoothstep.hpp>
#include <ccmath/math/misc/lerp.hpp>

#include <numbers>

namespace util {

inline float magnitude(sf::Vector2<float> vec) { return ccm::sqrt((vec.x * vec.x) + (vec.y * vec.y)); }
inline sf::Vector2<float> unit(sf::Vector2<float> vec) {
	auto denominator = ccm::sqrt((vec.x * vec.x) + (vec.y * vec.y));
	if (denominator == 0.f) { return sf::Vector2<float>{1.f, 0.f}; }
	return vec / denominator;
}
inline float direction(sf::Vector2<float> vec) { return atan2f(vec.y, vec.x); }
inline sf::Vector2<float> absolute_distance(sf::Vector2<float> source, sf::Vector2<float> destination) { return sf::Vector2<float>{abs(source.x - destination.x), abs(source.y - destination.y)}; }
	inline sf::Vector2<float> round_to_even(sf::Vector2<float> input) {
	auto ret = input;
	ret = {std::floor(ret.x / 2.f), std::floor(ret.y / 2.f)};
	auto intpos = static_cast<sf::Vector2<int>>(ret);
	ret = 2.f * static_cast<sf::Vector2<float>>(intpos);
	return ret;
}
inline float ease_in_out(float x, float y, float progress) { return ccm::ext::cubic(y, x, y, x, progress); }
inline float ease_out_back(float progress) {
	auto const c1{1.70158f};
	auto const c3 = c1 + 1.f;
	return 1.f + c3 * ccm::pow(progress - 1.f, 3.f) + c1 * ccm::pow(progress - 1.f, 2.f);
}
inline float smoothstep(float x, float y, float progress) { return ccm::ext::smoothstep(x, y, progress); }
inline bool same_parity(float a, float b) { return ((static_cast<int>(a) ^ static_cast<int>(b)) & 1) == 0; }
inline bool same_sign(float a, float b) { return a * b >= 0.f; }
inline uint8_t get_uint8_from_normal(float normal) { return static_cast<uint8_t>(ccm::lerp(0, 255, normal)); } // NOLINT

} // namespace util
