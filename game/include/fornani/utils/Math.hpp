
#pragma once

#include <SFML/Graphics.hpp>
#include <ccmath/ext/cubic.hpp>
#include <ccmath/ext/smoothstep.hpp>
#include <ccmath/math/basic/fabs.hpp>
#include <ccmath/math/misc/lerp.hpp>
#include <ccmath/math/nearest/floor.hpp>
#include <ccmath/math/power/sqrt.hpp>

#include <cmath>
#include <numbers>

namespace fornani::util {

constexpr auto f_pi{static_cast<float>(std::numbers::pi)};

enum class InterpolationType { linear, quadratic, cubic };

inline float magnitude(sf::Vector2f vec) { return ccm::sqrt((vec.x * vec.x) + (vec.y * vec.y)); }
inline sf::Vector2f unit(sf::Vector2f vec) {
	auto const denominator = ccm::sqrt((vec.x * vec.x) + (vec.y * vec.y));
	if (denominator == 0.f) { return sf::Vector2{1.f, 0.f}; }
	return vec / denominator;
}
// θ = atan(y / x)
inline float get_angle_from_direction(sf::Vector2f vec) { return ::std::atan2(vec.y, vec.x); } // TODO: Switch to ccm::atan2f when done.
// v = (cos(−θ), sin(−θ))
inline sf::Vector2f get_direction_from_angle(float angle) { return sf::Vector2f{::std::cos(-angle), ::std::sin(-angle)}; } // TODO: Switch to ccm::cos2f and ccm::sin2f when done.
inline sf::Vector2f absolute_distance(sf::Vector2f source, sf::Vector2f destination) { return sf::Vector2f{ccm::abs(source.x - destination.x), ccm::abs(source.y - destination.y)}; }
inline float round_to_nearest(float input, float target) { return target * static_cast<int>(ccm::floor(input / target)); }
inline float round_to_even(float input) { return 2.f * static_cast<int>(ccm::floor(input / 2.f)); }
inline sf::Vector2f round_to_even(sf::Vector2f input) {
	auto ret = input;
	ret = {ccm::floor(ret.x / 2.f), ccm::floor(ret.y / 2.f)};
	auto const intpos = static_cast<sf::Vector2<int>>(ret);
	ret = 2.f * static_cast<sf::Vector2f>(intpos);
	return ret;
}
inline sf::Vector2f round_up_to_even(sf::Vector2f input) {
	auto ret = input;
	ret = {std::ceil(ret.x / 2.f), std::ceil(ret.y / 2.f)};
	auto const intpos = static_cast<sf::Vector2<int>>(ret);
	ret = 2.f * static_cast<sf::Vector2f>(intpos);
	return ret;
}
inline float ease_in_out(float x, float y, float progress) { return ccm::ext::cubic(y, x, y, x, progress); }
inline float ease_out_back(float progress) {
	constexpr auto c1{1.70158f};
	auto const c3 = c1 + 1.f;
	// TODO: Once ccm::pow is done switch to that.
	return 1.f + c3 * std::pow(progress - 1.f, 3.f) + c1 * std::pow(progress - 1.f, 2.f);
}
inline float smoothstep(float const x, float const y, float const progress) { return ccm::ext::smoothstep(x, y, progress); }
inline bool same_parity(float const a, float const b) { return ((static_cast<int>(a) ^ static_cast<int>(b)) & 1) == 0; }
inline bool same_sign(float const a, float const b) { return a * b >= 0.f; }
inline std::uint8_t get_uint8_from_normal(float normal) { return static_cast<std::uint8_t>(ccm::lerp(0, 255, normal)); } // NOLINT
inline sf::Vector2f vector_lerp(sf::Vector2f const& a, sf::Vector2f const& b, float alpha) {
	auto t = std::clamp(alpha, 0.f, 1.f);
	return a + (b - a) * t;
}
inline sf::Vector2f arc_lerp_midpoint(sf::Vector2f const a, sf::Vector2f const b, float alpha, float direction = 1.f, sf::Vector2f const center_offset = {}) // +1 or -1
{
	float t = std::clamp(alpha, 0.f, 1.f);

	sf::Vector2f center = (a + b) * 0.5f + center_offset;
	sf::Vector2f v = a - center;
	float r = std::sqrt(v.x * v.x + v.y * v.y);
	float angleA = std::atan2(v.y, v.x);
	float angle = angleA + direction * f_pi * t;
	return center + sf::Vector2f(std::cos(angle) * r, std::sin(angle) * r);
}

} // namespace fornani::util
