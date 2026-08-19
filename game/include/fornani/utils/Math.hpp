
#pragma once

#include <SFML/Graphics.hpp>
#include <algorithm>
#include <cmath>
#include <numbers>

namespace bccm {
// borrowed from ccmath until the lib is fixed
template <typename T, std::enable_if_t<std::is_floating_point_v<T>, bool> = true>
constexpr T smoothstep(T edge0, T edge1, T x) {
	// Scale, bias and saturate x to 0..1 range
	x = std::clamp((x - edge0) / (edge1 - edge0), static_cast<T>(0), static_cast<T>(1));
	// Evaluate polynomial
	return x * x * (static_cast<T>(3) - static_cast<T>(2) * x);
}
template <typename T, std::enable_if_t<std::is_floating_point_v<T>, bool> = true>
constexpr T cubic(T y0, T y1, T y2, T y3, T t) {
	T const a0 = y3 - y2 - y0 + y1;
	T const a1 = y0 - y1 - a0;
	T const a2 = y2 - y0;
	T const a3 = y1;

	T const t2 = t * t;

	return (a0 * t * t2) + (a1 * t2) + (a2 * t) + a3;
}

} // namespace bccm

namespace fornani::util {

constexpr auto f_pi{static_cast<float>(std::numbers::pi)};

enum class InterpolationType : std::uint8_t { linear, quadratic, cubic, ease };

static float dot(sf::Vector2f a, sf::Vector2f b) { return a.x * b.x + a.y * b.y; }
inline float magnitude(sf::Vector2f vec) { return std::sqrt((vec.x * vec.x) + (vec.y * vec.y)); }
inline sf::Vector2f unit(sf::Vector2f vec) {
	auto const denominator = std::sqrt((vec.x * vec.x) + (vec.y * vec.y));
	if (denominator == 0.f) { return sf::Vector2{1.f, 0.f}; }
	return vec / denominator;
}
// θ = atan(y / x)
inline float get_angle_from_direction(sf::Vector2f vec) { return ::std::atan2(vec.y, vec.x); } // TODO: Switch to ccm::atan2f when done.
// v = (cos(−θ), sin(−θ))
inline sf::Vector2f get_direction_from_angle(float angle) { return sf::Vector2f{::std::cos(-angle), ::std::sin(-angle)}; } // TODO: Switch to ccm::cos2f and ccm::sin2f when done.
inline sf::Vector2f absolute_distance(sf::Vector2f source, sf::Vector2f destination) { return sf::Vector2f{std::abs(source.x - destination.x), std::abs(source.y - destination.y)}; }
inline float round_to_nearest(float input, float target) { return target * static_cast<int>(std::floor(input / target)); }
inline float round_to_even(float input) { return 2.f * static_cast<int>(std::floor(input / 2.f)); }
inline sf::Vector2f round_to_even(sf::Vector2f input) { return 2.f * sf::Vector2f{std::round(input.x / 2.f), std::round(input.y / 2.f)}; }
inline sf::Vector2f round_to(sf::Vector2f input, float factor) {
	if (factor == 0.f) { return sf::Vector2f{}; }
	return factor * sf::Vector2f{std::round(input.x / factor), std::round(input.y / factor)};
}
inline sf::Vector2f round_up_to_even(sf::Vector2f input) {
	auto ret = input;
	ret = {std::ceil(ret.x / 2.f), std::ceil(ret.y / 2.f)};
	auto const intpos = static_cast<sf::Vector2<int>>(ret);
	ret = 2.f * static_cast<sf::Vector2f>(intpos);
	return ret;
}
inline float slowdown(float t) { return 1.0f - 4.0f * (t - 0.5f) * (t - 0.5f); }
inline float ease_in_out(float x, float y, float progress) { return bccm::cubic(y, x, y, x, progress); }
inline float ease_out_back(float progress) {
	constexpr auto c1{1.70158f};
	auto const c3 = c1 + 1.f;
	// TODO: Once ccm::pow is done switch to that.
	return 1.f + c3 * std::pow(progress - 1.f, 3.f) + c1 * std::pow(progress - 1.f, 2.f);
}
inline float smoothstep(float const x, float const y, float const progress) { return bccm::smoothstep(x, y, progress); }
inline float smoothstep(float x) { return x * x * (3.0f - 2.0f * x); }
inline float smootherstep(float x) { return x * x * x * (x * (x * 6.0f - 15.0f) + 10.0f); }
inline bool same_parity(float const a, float const b) { return ((static_cast<int>(a) ^ static_cast<int>(b)) & 1) == 0; }
inline bool same_sign(float const a, float const b) { return a * b >= 0.f; }
inline std::uint8_t get_uint8_from_normal(float normal) { return static_cast<std::uint8_t>(std::lerp(0, 255, normal)); } // NOLINT
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
inline int map_to_frame(float value, float min, float max, int min_frame, int max_frame) {
	float v = std::clamp(value, min, max);
	float t = (v - min) / (max - min);
	float frame_float = min_frame + t * (max_frame - min_frame);
	return static_cast<int>(std::round(frame_float));
}

} // namespace fornani::util
