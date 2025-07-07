
#pragma once

#include <SFML/Graphics.hpp>

namespace pi {

enum class SelectionType { neutral, palette, canvas };

class SelectBox {
  public:
	SelectBox(sf::Vector2<std::uint32_t> pos, sf::Vector2<std::uint32_t> dim, SelectionType type) : position(pos), dimensions(dim), type(type) {}
	void adjust(sf::Vector2<std::uint32_t> adjustment) { dimensions = adjustment; }
	[[nodiscard]] auto get_type() const -> SelectionType { return type; }
	[[nodiscard]] auto f_position() const -> sf::Vector2f { return {static_cast<float>(position.x), static_cast<float>(position.y)}; }
	[[nodiscard]] auto empty() const -> bool { return dimensions.x * dimensions.y == 0; }
	sf::Vector2<std::uint32_t> position{};
	sf::Vector2<std::uint32_t> dimensions{};

  private:
	SelectionType type{};
};

} // namespace pi
