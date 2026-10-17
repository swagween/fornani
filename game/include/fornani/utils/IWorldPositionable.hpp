
#pragma once

#include <SFML/Graphics.hpp>
#include <fornani/utils/Constants.hpp>

namespace fornani {

class IWorldPositionable {
  public:
	explicit IWorldPositionable(sf::Vector2i const pos, sf::Vector2u const dim = {1U, 1U}) : m_position{pos}, m_dimensions{dim} {}
	void set_grid_position(sf::Vector2i const to) { m_position = to; }
	void set_grid_position(sf::Vector2u const to) { m_position = sf::Vector2i{to}; }
	void set_grid_dimensions(sf::Vector2u const to) { m_dimensions = to; }
	void set_world_position(sf::Vector2f const to) { m_position = sf::Vector2i{to / constants::f_cell_size}; }
	void set_world_dimensions(sf::Vector2f const to) { m_dimensions = sf::Vector2u{to / constants::f_cell_size}; }

	[[nodiscard]] auto get_grid_position() const -> sf::Vector2i { return m_position; }
	[[nodiscard]] auto get_f_grid_position() const -> sf::Vector2f { return sf::Vector2f{m_position}; }
	[[nodiscard]] auto get_grid_dimensions() const -> sf::Vector2u { return m_dimensions; }
	[[nodiscard]] auto get_f_grid_dimensions() const -> sf::Vector2f { return sf::Vector2f{m_dimensions}; }
	[[nodiscard]] auto get_world_position() const -> sf::Vector2f { return sf::Vector2f{m_position} * constants::f_cell_size; }
	[[nodiscard]] auto get_world_dimensions() const -> sf::Vector2f { return sf::Vector2f{m_dimensions} * constants::f_cell_size; }
	[[nodiscard]] auto get_local_center() const -> sf::Vector2f { return get_world_dimensions() * 0.5f; }
	[[nodiscard]] auto get_global_center() const -> sf::Vector2f { return get_world_position() + get_local_center(); }
	[[nodiscard]] auto is_large() const -> bool { return m_dimensions.x > 1U || m_dimensions.y > 1U; }

  protected:
	sf::Vector2i m_position{};
	sf::Vector2u m_dimensions{};
};

} // namespace fornani
