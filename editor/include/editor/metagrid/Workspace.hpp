
#pragma once

#include <SFML/Graphics.hpp>
#include <editor/metagrid/Room.hpp>
#include <fornani/graphics/Colors.hpp>
#include <fornani/io/Logger.hpp>

namespace pi {

constexpr auto metagrid_dimensions_v = sf::Vector2u{512, 512};

struct Point {
  public:
	Point(int one_d_index, sf::Vector2u dim) : m_position{one_d_index % static_cast<int>(dim.x), one_d_index / static_cast<int>(dim.x)} {
		dot.setFillColor(sf::Color{48, 35, 7});
		dot.setSize({2.f, 2.f});
		dot.setPosition(get_board_position());
	}
	[[nodiscard]] auto get_board_position() const -> sf::Vector2f { return sf::Vector2f{m_position} * spacing_v; }
	[[nodiscard]] auto get_f_index_position() const -> sf::Vector2f { return sf::Vector2f{m_position}; }
	[[nodiscard]] auto get_index_position() const -> sf::Vector2i { return m_position; }

	sf::RectangleShape dot{};

  private:
	sf::Vector2i m_position{};
};

class Workspace {
  public:
	Workspace(sf::Vector2u dimensions = metagrid_dimensions_v);

	void render(sf::RenderWindow& win, sf::Vector2f cam);
	void zoom(float const factor);

  private:
	std::vector<Point> m_points{};
	sf::RenderTexture m_texture{};

	fornani::io::Logger m_logger{"Editor"};
};

} // namespace pi
