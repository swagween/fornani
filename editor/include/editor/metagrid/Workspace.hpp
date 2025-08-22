
#pragma once

#include <SFML/Graphics.hpp>
#include <editor/metagrid/Room.hpp>
#include <fornani/graphics/Colors.hpp>

namespace pi {
constexpr auto metagrid_dimensions_v = sf::Vector2u{512, 512};

struct Point {
  public:
	Point(int one_d_index, sf::Vector2u dim) : m_position{one_d_index % static_cast<int>(dim.x), one_d_index / static_cast<int>(dim.x)} {
		dot.setFillColor(fornani::colors::dark_grey);
		dot.setSize({2.f, 2.f});
		dot.setOrigin({1.f, 1.f});
	}
	[[nodiscard]] auto get_board_position() const -> sf::Vector2f { return sf::Vector2f{m_position} * spacing_v; }
	[[nodiscard]] auto get_f_index_position() const -> sf::Vector2f { return sf::Vector2f{m_position}; }
	[[nodiscard]] auto get_index_position() const -> sf::Vector2i { return m_position; }

	void render(sf::RenderWindow& win, sf::Vector2f cam) {
		dot.setPosition(get_board_position() + cam);
		win.draw(dot);
	}

  private:
	sf::Vector2i m_position{};
	sf::RectangleShape dot{};
};

class Workspace {
  public:
	Workspace(sf::Vector2u dimensions = metagrid_dimensions_v);

	void render(sf::RenderWindow& win, sf::Vector2f cam);

  private:
	std::vector<Point> m_points{};
};

} // namespace pi
