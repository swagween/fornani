//
//  Tile.hpp
//  for_loop
//
//  Created by Alex Frasca on 10/6/18.
//  Copyright © 2018 Western Forest Studios. All rights reserved.
//

#ifndef Tile_hpp
#define Tile_hpp

#include "../utils/shape.hpp"
#include <vector>

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

class Tile {
    
public:
    
    Tile();
    Tile(int x, int y, int w, int h, uint8_t value);
    
    uint8_t value; //for figuring out which texture to use
    
    TILE_TYPE type; //for assigning attributes
    Shape bounding_box; //for collision
    
};

#endif /* Tile_hpp */
