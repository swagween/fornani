
#pragma once

#include "editor/util/SelectBox.hpp"

#include <SFML/Graphics.hpp>

#include <cassert>
#include <optional>
#include <vector>

namespace pi {

enum class SelectMode : std::uint8_t { none, select, clipboard };

class Canvas;
class Tool;

class Clipboard {

  public:
	Clipboard(sf::Vector2<std::uint32_t> dimensions);
	void write_to_clipboard(std::uint32_t value, size_t i, size_t j, size_t layer);
	void clear_clipboard();
	void cut(Canvas& canvas, Tool& tool);
	void copy(Canvas& canvas, Tool& tool);
	void paste(Canvas& canvas, Tool& tool);
	void render(Canvas& canvas, Tool& tool, sf::RenderWindow& win, sf::Vector2<float> offset);
	std::uint32_t get_value_at(size_t i, size_t j);
	std::uint32_t get_value_at(size_t i, size_t j, size_t layer);
	[[nodiscard]] auto empty() const -> bool { return m_cell_values.empty(); }
	[[nodiscard]] auto real_dimensions() const -> sf::Vector2<float> { return {static_cast<float>(m_dimensions.x), static_cast<float>(m_dimensions.y)}; }
	[[nodiscard]] auto scaled_dimensions() const -> sf::Vector2<std::uint32_t> { return m_dimensions; }

  private:
	sf::Vector2<std::uint32_t> m_dimensions{};
	std::vector<std::vector<std::uint32_t>> m_cell_values{};
	SelectMode m_mode{};
	sf::RectangleShape m_box{};
	SelectionType source{};
};

} // namespace pi
