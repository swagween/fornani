//
//  Map.hpp
//  for_loop
//
//  Created by Alex Frasca on 10/6/18.
//  Copyright © 2018 Western Forest Studios. All rights reserved.
//
#pragma once

#include "../utils/Grid.hpp"
#include "../utils/Shape.hpp"
//#include "../components/PhysicsComponent.hpp"
#include <string>
#include <vector>
#include <fstream>

const int NUM_LAYERS = 8;

namespace world {

enum LAYER_ORDER {
    BACKGROUND = 0,
    MIDDLEGROUND = 4,
    FOREGROUND = 7,
};

// a Layer is a grid with a render priority and a flag to determine if scene entities can collide with it.
// for for loop, the current convention is that the only collidable layer is layer 4 (index 3), or the middleground.

class Layer {
    
public:
    
    Layer() = default;
    Layer(uint8_t o, bool c) : render_order(o), collidable(c) {
        grid = squid::Grid({48, 32});
    }
    squid::Grid grid{};
    uint8_t render_order{};
    bool collidable{};
    
};

// a Map is just a set of layers that will render on top of each other

class Map {
    
public:
    
    Map();
    
    //methods
    void load(const std::string& path);
    void update();
    squid::Tile* tile_at(const uint8_t i, const uint8_t j);
    Shape* shape_at(const uint8_t i, const uint8_t j);
    
    //layers
    std::vector<Layer> layers;
    
};

}
/* Map_hpp */
