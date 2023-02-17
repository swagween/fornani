//
//  Map.cpp
//  for_loop
//
//  Created by Alex Frasca on 10/6/18.
//  Copyright © 2018 Western Forest Studios. All rights reserved.
//

#include "Map.hpp"
#include "../setup/ServiceLocator.hpp"

namespace world {

Map::Map() {
    for(int i = 0; i < NUM_LAYERS; ++i) {
        layers.push_back(Layer( i, (i == MIDDLEGROUND) ));
        layers.back().grid.initialize();
    }
}

void Map::load(const std::string& path) {
    for(int i = 0; i < NUM_LAYERS; ++i) {
        layers.push_back(Layer( i, (i == MIDDLEGROUND) ));
        layers.back().grid.initialize();
    }
    
    //get data from text files
    int value{};
    int counter = 0;
    std::ifstream input{};
    for(auto& layer : layers) {
        //open map_tiles_[i].txt
        input.open(path + "/map_tiles_" + std::to_string(counter) + ".txt");
        for(auto& cell : layer.grid.cells) {
            input >> value;
            squid::TILE_TYPE typ = squid::lookupType(value);
            cell.value = value;
            cell.type = typ;
            
            input.ignore(); //ignore the delimiter
            input.ignore();
            input.ignore();
        }
        layer.grid.update();
        layer.grid.init_shape_vertices();
        //close the current file
        input.close();
        ++counter;
    }
    
}

void Map::update() {
    svc::playerLocator.get().is_any_jump_colllision = false;
    svc::playerLocator.get().is_any_colllision = false;
    svc::playerLocator.get().left_aabb_counter = 0;
    svc::playerLocator.get().right_aabb_counter = 0;
    
    //someday, I will have a for(auto& entity : entities) loop and the player will be included in that
    for(auto& cell : layers.at(MIDDLEGROUND).grid.cells) {
        if(abs(cell.bounding_box.shape_x - svc::playerLocator.get().hurtbox.shape_x) > PLAYER_WIDTH*1.5f ||
           abs(cell.bounding_box.shape_y - svc::playerLocator.get().hurtbox.shape_y) > PLAYER_HEIGHT*1.5f) {
            cell.collision_check = false;
            continue;
        } else {
            cell.collision_check = true;
            
            if(cell.value > 0) {
                svc::playerLocator.get().handle_map_collision(cell.bounding_box, cell.type == squid::TILE_RAMP);
            }
        }
    }
    //update player flags
    if(svc::playerLocator.get().left_aabb_counter == 0) {
        svc::playerLocator.get().has_left_collision = false;
    }
    if(svc::playerLocator.get().right_aabb_counter == 0) {
        svc::playerLocator.get().has_right_collision = false;
    }
    if(svc::playerLocator.get().is_any_jump_colllision) {
        svc::playerLocator.get().grounded = true;
    } else {
        svc::playerLocator.get().grounded = false;
    }
    if(svc::playerLocator.get().is_any_colllision) {
        svc::playerLocator.get().is_colliding_with_level = true;
    } else {
        svc::playerLocator.get().is_colliding_with_level = false;
    }
}

squid::Tile* Map::tile_at(const uint8_t i, const uint8_t j) {
    //for checking tile value
    if(i * j < layers.at(MIDDLEGROUND).grid.cells.size()) {
        return &layers.at(MIDDLEGROUND).grid.cells.at(i + j * layers.at(MIDDLEGROUND).grid.dimensions.x);
    }
}

Shape* Map::shape_at(const uint8_t i, const uint8_t j) {
    //for testing collision
    if(i * j < layers.at(MIDDLEGROUND).grid.cells.size()) {
        return &layers.at(MIDDLEGROUND).grid.cells.at(i + j * layers.at(MIDDLEGROUND).grid.dimensions.x).bounding_box;
    }
}

}// End world
