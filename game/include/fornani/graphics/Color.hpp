
#pragma once

#include <SFML/Graphics.hpp>
#include <djson/json.hpp>
#include <cstdint>

namespace fornani {

class Color : public sf::Color {
  public:
	Color() = default;
	Color(dj::Json const& in) : sf::Color{in[0].as<std::uint8_t>(), in[1].as<std::uint8_t>(), in[2].as<std::uint8_t>(), in.as_array().size() > 3 ? in[3].as<std::uint8_t>() : std::uint8_t{255}} {}
};

static inline sf::Color color_lerp(sf::Color a, sf::Color b, float t) {
	auto lerp_channel = [t](std::uint8_t x, std::uint8_t y) { return static_cast<std::uint8_t>(std::round(std::lerp(float(x), float(y), t))); };

	return {lerp_channel(a.r, b.r), lerp_channel(a.g, b.g), lerp_channel(a.b, b.b), lerp_channel(a.a, b.a)};
}

static inline sf::Color gradient_color(std::span<sf::Color const> colors, float alpha) {
	assert(!colors.empty());

	alpha = std::clamp(alpha, 0.0f, 1.0f);

	if (colors.size() == 1) return colors.front();

	float const position = alpha * (colors.size() - 1);

	auto const left = static_cast<std::size_t>(std::floor(position));
	auto const right = std::min(left + 1, colors.size() - 1);

	float const t = position - left;

	return color_lerp(colors[left], colors[right], t);
}

} // namespace fornani
