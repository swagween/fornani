
#pragma once

#include <SFML/Graphics.hpp>
#include <fornani/utils/Constants.hpp>

namespace fornani {

class IWorldPositionable {
  public:
	explicit IWorldPositionable(sf::Vector2<std::uint32_t> pos, sf::Vector2<std::uint32_t> dim = constants::u32_cell_vec) : m_position{pos}, m_dimensions{dim} {}

	void set_grid_position(sf::Vector2<std::uint32_t> to) { m_position = to; }
	void set_grid_position(sf::Vector2i to) { m_position = sf::Vector2<std::uint32_t>{to}; }
	void set_grid_dimensions(sf::Vector2<std::uint32_t> to) { m_dimensions = to; }
	void set_grid_dimensions(sf::Vector2i to) { m_dimensions = sf::Vector2<std::uint32_t>{to}; }
	void set_world_position(sf::Vector2f to) { m_position = sf::Vector2<std::uint32_t>{to / constants::f_cell_size}; }
	void set_world_dimensions(sf::Vector2f to) { m_dimensions = sf::Vector2<std::uint32_t>{to / constants::f_cell_size}; }

	[[nodiscard]] auto get_grid_position() const -> sf::Vector2<std::uint32_t> { return m_position; }
	[[nodiscard]] auto get_f_grid_position() const -> sf::Vector2f { return sf::Vector2f{m_position}; }
	[[nodiscard]] auto get_grid_dimensions() const -> sf::Vector2<std::uint32_t> { return m_dimensions; }
	[[nodiscard]] auto get_f_grid_dimensions() const -> sf::Vector2f { return sf::Vector2f{m_dimensions}; }
	[[nodiscard]] auto get_world_position() const -> sf::Vector2f { return sf::Vector2f{m_position} * constants::f_cell_size; }
	[[nodiscard]] auto get_world_dimensions() const -> sf::Vector2f { return sf::Vector2f{m_dimensions} * constants::f_cell_size; }
	[[nodiscard]] auto get_local_center() const -> sf::Vector2f { return get_world_dimensions() * 0.5f; }
	[[nodiscard]] auto get_global_center() const -> sf::Vector2f { return get_world_position() + get_local_center(); }
	[[nodiscard]] auto is_large() const -> bool { return get_grid_dimensions().x * get_grid_dimensions().y > 1; }

  protected:
	sf::Vector2<std::uint32_t> m_position{};
	sf::Vector2<std::uint32_t> m_dimensions{};
};

} // namespace fornani
