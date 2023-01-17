//
//  Grid.cpp
//  squid
//
//  Created by Alex Frasca on 12/26/22.
//

#include "Grid.hpp"

squid::Grid::Grid() {
    spacing = DEFAULT_SPACING;
}

squid::Grid::Grid(sf::Vector2<uint32_t> d) : dimensions(d) {
    
    spacing = DEFAULT_SPACING;
    
    initialize();
    
}

void squid::Grid::initialize() {
    
    cells.clear();
    
    for(int i = 0; i < dimensions.x * dimensions.y; i++) {
        
        // calculate index values
        uint32_t xidx = std::floor(i%dimensions.x);
        uint32_t yidx = std::floor(i/dimensions.x);
        
        //calculate positions with offsets
        float xpos, ypos;
        if(yidx % 2 == 0) {
            xpos = xidx*spacing;
        } else {
            xpos = xidx*spacing;
        }
        
        if(xidx % 2 == 0) {
            ypos = yidx*spacing;
        } else {
            ypos = yidx*spacing;
        }
        cells.push_back(Tile({xidx, yidx}, {xpos, ypos}, 0, TILE_NULL));
        
    }
    
}

void squid::Grid::update() {
    
    for(int i = 0; i < dimensions.x * dimensions.y; i++) {
        
        pushCells(i);
        
    }
    
}

void squid::Grid::setSpacing(float spc) {
    
    spacing = spc;
    for(int i = 0; i < dimensions.x * dimensions.y; i++) {
        float xpos = cells.at(i).index.x * spacing;
        float ypos = cells.at(i).index.y * spacing;
        cells.at(i).position = sf::Vector2<float>(xpos, ypos);
    }
    
}

void squid::Grid::pushCells(int i) {
    // calculate index values
    uint32_t xidx = std::floor(i%dimensions.x);
    uint32_t yidx = std::floor(i/dimensions.x);
    
    //calculate positions with offsets
    float xpos, ypos;
    if(yidx % 2 == 0) {
        xpos = xidx*spacing;
    } else {
        xpos = xidx*spacing;
    }
    
    if(xidx % 2 == 0) {
        ypos = yidx*spacing;
    } else {
        ypos = yidx*spacing;
    }
    
    //populate the grid
    cells.at(i).index = sf::Vector2<uint32_t>(xidx, yidx);
    cells.at(i).position = sf::Vector2<float>(xpos, ypos);
    
    cells.at(i).bounding_box.update(xpos, ypos, spacing, spacing);
}

squid::TILE_TYPE squid::lookupType(int idx) {
    if(idx < 1) {
        return TILE_NULL;
    }
    if(idx < 192) {
        return TILE_BASIC;
    }
    if(idx <= 223) {
        return TILE_RAMP;
    }
    if(idx <= 227) {
        return TILE_LAVA;
    }
    if(idx <= 231) {
        return TILE_CURRENT;
    }
    if(idx <= 235) {
        return TILE_FLAMMABLE;
    }
    if(idx <= 239) {
        return TILE_PLATFORM;
    }
    if(idx <= 243) {
        return TILE_WATER;
    }
    if(idx <= 247) {
        return TILE_BREAKABLE;
    }
    if(idx <= 251) {
        return TILE_LADDER;
    }
    if(idx <= 255) {
        return TILE_SPIKES;
    }
    return TILE_NULL;
}
