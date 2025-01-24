//
//  Tile.hpp
//  Pioneer-Lab
//
//  Created by Alex Frasca on 9/30/20.
//

#pragma once

#include <SFML/Graphics.hpp>
#include <stdio.h>

namespace pi {

enum TILE_TYPE {
    TILE_NULL = -1,
    TILE_BASIC = 0,
    TILE_RAMP,
    TILE_LAVA,
    TILE_CURRENT,
    TILE_FLAMMABLE,
    TILE_PLATFORM,
    TILE_WATER,
    TILE_BREAKABLE,
    TILE_LADDER,
    TILE_SPIKES,
    TILE_ICY
};

const int CEIL_SLANT_INDEX = 192;
const int FLOOR_SLANT_INDEX = 208;

struct Tile {
    
    Tile() {};
    Tile(sf::Vector2<uint32_t> i, sf::Vector2<float> p, uint32_t val, TILE_TYPE t) : index(i), position(p), value(val), type(t) {}
	void set_scale(float to_scale) { scale = to_scale; }
	[[nodiscard]] auto scaled_position() const -> sf::Vector2<float> { return position * scale; }
	[[nodiscard]] auto grid_position() const -> sf::Vector2<int> { return sf::Vector2<int>{position / 32.f}; }
	[[nodiscard]] auto set_position(sf::Vector2<float> to_position) { position = to_position; }
    
    sf::Vector2<uint32_t> index{};
    
    uint32_t value{};
    TILE_TYPE type{}; //for assigning attributes
    
    bool collision_check{};

  private:
	float scale{};
	sf::Vector2<float> position{};
    
};

}
