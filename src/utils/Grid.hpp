//
//  Grid.hpp
//  squid
//
//  Created by Alex Frasca on 12/26/22.
//
#pragma once

#include "../utils/Shape.hpp"
#include <stdio.h>
#include <vector>
#include <cmath>

namespace squid {

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

const float DEFAULT_SPACING = 32.0;

const int CEIL_SLANT_INDEX = 192;
const int FLOOR_SLANT_INDEX = 208;

struct Tile {
    
    Tile() {};
    Tile(sf::Vector2<uint32_t> i, sf::Vector2<float> p, uint32_t val, TILE_TYPE t) : index(i), position(p), value(val), type(t) {}
    
    sf::Vector2<uint32_t> index{};
    sf::Vector2<float> position{};
    
    uint32_t value{};
    TILE_TYPE type{}; //for assigning attributes
    Shape bounding_box{}; //for collision
    
    bool collision_check{};

};

class Grid {
public:
    
    Grid();
    Grid(sf::Vector2<uint32_t> d);
    
    sf::Vector2<uint32_t> dimensions{};
    float spacing{};
    
    std::vector<Tile> cells;
    
    void initialize();
    void update();
    
    void init_shape_vertices();
    
    void set_spacing(float spc);
    
    void push_cells(int i);
    
};

TILE_TYPE lookup_type(int idx);

} // end squid

/* Grid_hpp */
