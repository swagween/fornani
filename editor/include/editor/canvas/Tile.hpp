
#pragma once

#include <SFML/Graphics.hpp>
#include <stdio.h>
#include "fornani/world/Tile.hpp"

namespace pi {

struct Tile {
    
    Tile() {};
    Tile(sf::Vector2<uint32_t> i, sf::Vector2<float> p, uint32_t val, uint32_t odi) : index(i), position(p), value(val), one_d_index(odi) {}
	void set_scale(float to_scale) { scale = to_scale; }
	[[nodiscard]] auto scaled_position() const -> sf::Vector2<float> { return position * scale; }
	[[nodiscard]] auto grid_position() const -> sf::Vector2<int> { return sf::Vector2<int>{position / 32.f}; }
	[[nodiscard]] auto set_position(sf::Vector2<float> to_position) { position = to_position; }
	[[nodiscard]] auto is_solid() const -> bool { return value > 0 && value < fornani::world::special_index_v; }
    
    sf::Vector2<uint32_t> index{};
	uint32_t one_d_index{};
    uint32_t value{};
    
    bool collision_check{};

  private:
	float scale{};
	sf::Vector2<float> position{};
    
};

}
