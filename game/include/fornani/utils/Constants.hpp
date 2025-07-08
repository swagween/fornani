
#pragma once
#include <SFML/Graphics.hpp>

namespace fornani::constants {

constexpr std::uint8_t u_scale_factor{2u};
constexpr int i_scale_factor{static_cast<int>(u_scale_factor)};
constexpr float f_scale_factor{static_cast<float>(u_scale_factor)};

constexpr std::uint8_t u_cell_resolution{16};
constexpr unsigned int ui_cell_resolution{static_cast<unsigned int>(u_cell_resolution)};
constexpr int i_cell_resolution{static_cast<int>(u_cell_resolution)};
constexpr float f_cell_resolution{static_cast<float>(i_cell_resolution)};

constexpr std::uint8_t u8_cell_size{u_cell_resolution * u_scale_factor};
constexpr std::uint32_t u32_cell_size{static_cast<std::uint32_t>(u8_cell_size)};
constexpr int i_cell_size{static_cast<int>(u32_cell_size)};
constexpr float f_cell_size{static_cast<float>(u32_cell_size)};

constexpr sf::Vector2<std::uint32_t> u32_cell_vec{u32_cell_size, u32_cell_size};
constexpr sf::Vector2i i_cell_vec{i_cell_size, i_cell_size};
constexpr sf::Vector2f f_cell_vec{f_cell_size, f_cell_size};

constexpr sf::Vector2i i_resolution_vec{i_cell_resolution, i_cell_resolution};
constexpr sf::Vector2f f_resolution_vec{f_cell_resolution, f_cell_resolution};
constexpr sf::Vector2i i_scale_vec{i_scale_factor, i_scale_factor};
constexpr sf::Vector2f f_scale_vec{f_scale_factor, f_scale_factor};
constexpr sf::Vector2f f_inverse_scale_vec{-f_scale_factor, f_scale_factor};

constexpr sf::Vector2i tileset_dimensions{16, 32};

constexpr int i_chunk_size{16};
constexpr float f_chunk_size{static_cast<float>(i_chunk_size)};

} // namespace fornani::constants
