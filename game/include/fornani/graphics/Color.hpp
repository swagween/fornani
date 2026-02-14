
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

} // namespace fornani
