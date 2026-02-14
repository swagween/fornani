
#pragma once

#include <SFML/Graphics.hpp>
#include <fornani/graphics/Colors.hpp>
#include <utility>
#include <vector>

namespace fornani::gui {

struct Dot {
	sf::RectangleShape rect{};
	sf::Vector2f position{};
};

struct DotProperties {
	sf::Color color{};
	float size{};
};

class DottedLine {
  public:
	DottedLine(std::pair<sf::Vector2f, sf::Vector2f> segment, float spacing, DotProperties properties = {colors::ui_white, 4.f});
	void update();
	void render(sf::RenderWindow& win, float scale, sf::Vector2f position, sf::Vector2f viewport_size);

  private:
	void create_dot(sf::Vector2f const position, DotProperties const& properties);
	std::vector<Dot> m_dots{};
	std::pair<sf::Vector2f, sf::Vector2f> m_segment;
	DotProperties m_properties;
	float m_spacing;
};

} // namespace fornani::gui
