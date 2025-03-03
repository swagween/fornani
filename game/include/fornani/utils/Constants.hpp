
#pragma once
#include <SFML/Graphics.hpp>

namespace fornani::util {

struct Constants {
	int const i_texture_scale{2};
	float const f_texture_scale{2.f};
	sf::Vector2<float> const texture_scale{f_texture_scale, f_texture_scale};
	int const i_texture_cell_size{16};
	float const f_texture_cell_size{16.0f};
	sf::Vector2<int> const tileset_scaled{i_texture_cell_size, i_texture_cell_size};
	uint32_t const u32_cell_size{static_cast<uint32_t>(i_texture_cell_size * i_texture_scale)};
	int const i_cell_size{i_texture_cell_size * i_texture_scale};
	float const cell_size{f_texture_cell_size * f_texture_scale};
	sf::Vector2<int> const i_cell_vec{i_cell_size, i_cell_size};
	sf::Vector2<float> const f_cell_vec{cell_size, cell_size};
	float const chunk_size{16.0f};
	int const i_chunk_size{16};
	sf::Vector2<int> const aspect_ratio{3840, 2048};

	// TODO: This prob should be in the windowing class.
	// screen constants (non-const because they must be set after window creation)
	sf::Vector2<int> screen_dimensions{aspect_ratio.x / 4, aspect_ratio.y / 4};
	sf::Vector2<float> f_screen_dimensions{static_cast<float>(aspect_ratio.x) / 4.f, static_cast<float>(aspect_ratio.y) / 4.f};
	sf::Vector2<float> f_center_screen{f_screen_dimensions * 0.5f};
	void set_screen_constants(sf::Vector2<int> dimensions) {
		screen_dimensions = dimensions;
		f_screen_dimensions = {static_cast<float>(dimensions.x), static_cast<float>(dimensions.y)};
		f_center_screen = {f_screen_dimensions * 0.5f};
	}
};

} // namespace fornani::util
