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
    }
}

void Map::load(const std::string& path) {
    for(int i = 0; i < NUM_LAYERS; ++i) {
        layers.push_back(Layer( i, (i == MIDDLEGROUND) ));
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
        //close the current file
        input.close();
        ++counter;
    }
    
}

void Map::update() {
    for(auto& cell : layers.at(MIDDLEGROUND).grid.cells) {
        if(abs(cell.bounding_box.shape_x - svc::playerLocator.get().hurtbox.shape_x) > PLAYER_WIDTH*1.5 ||
           abs(cell.bounding_box.shape_y - svc::playerLocator.get().hurtbox.shape_y) > PLAYER_HEIGHT*1.5) {
            cell.collision_check = false;
            continue;
        } else {
            cell.collision_check = true;
            if(svc::playerLocator.get().hurtbox.SAT(cell.bounding_box)) {
                if(cell.value > 0) {
                    sf::operator+=(svc::playerLocator.get().mtv, svc::playerLocator.get().hurtbox.testCollisionGetMTV(svc::playerLocator.get().hurtbox, cell.bounding_box));
                    if(abs(svc::playerLocator.get().mtv.y) > 1.0f) {
                        svc::playerLocator.get().physics.velocity.y = 0.0f;
                    }
                    if(abs(svc::playerLocator.get().mtv.x) > 1.0f) {
                        svc::playerLocator.get().physics.velocity.x = 0.0f;
                    }
                    svc::playerLocator.get().physics.acceleration = {0.0f, 0.0f};
                }
            }
        }
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
