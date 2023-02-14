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
    bool is_any_jump_colllision = false;
    bool is_any_colllision = false;
    for(auto& cell : layers.at(MIDDLEGROUND).grid.cells) {
        if(abs(cell.bounding_box.shape_x - svc::playerLocator.get().hurtbox.shape_x) > PLAYER_WIDTH*1.0f ||
           abs(cell.bounding_box.shape_y - svc::playerLocator.get().hurtbox.shape_y) > PLAYER_HEIGHT*1.5f) {
            cell.collision_check = false;
            continue;
        } else {
            cell.collision_check = true;
            if(svc::playerLocator.get().predictive_hurtbox.SAT(cell.bounding_box)) {
                if(cell.value > 0) {
                    is_any_colllision = true;
                    svc::playerLocator.get().physics.mtv = svc::playerLocator.get().predictive_hurtbox.testCollisionGetMTV(svc::playerLocator.get().predictive_hurtbox, cell.bounding_box);
                    
                    
                    if(svc::playerLocator.get().physics.velocity.y > 3.0f) {
                        svc::playerLocator.get().physics.mtv.x = 0.0f;
                    }
                    //here, we can do MTV again with the player's predicted position based on velocity
                    
                    
                    //resolve the collision here to prevent 1-frame clipping
                    //all of this stuff is necessary
                    
                    //calculate how much we need to move the player once there's an overlap with predictive_hurtbox
                    //get distance
                    if(svc::playerLocator.get().physics.velocity.y > -0.01f) {
                        float ydist = svc::playerLocator.get().predictive_hurtbox.shape_y - svc::playerLocator.get().physics.position.y;
                        float correction = ydist + svc::playerLocator.get().physics.mtv.y;
                        svc::playerLocator.get().physics.position.y += correction;
                        svc::playerLocator.get().physics.velocity.y = 0.0f;
                        svc::playerLocator.get().physics.acceleration.y = 0.0f;
                    }
                    if(abs(svc::playerLocator.get().physics.mtv.x) > 0.01f && cell.type != squid::TILE_RAMP) {
                        svc::playerLocator.get().physics.position.x += svc::playerLocator.get().physics.mtv.x;
                        svc::playerLocator.get().physics.velocity.x = 0.0f;
                        svc::playerLocator.get().physics.acceleration.x = 0.0f;
                        if(svc::playerLocator.get().physics.mtv.x > 0.0f) {
                            svc::playerLocator.get().has_left_collision = true;
                        }
                        if(svc::playerLocator.get().physics.mtv.x < 0.0f) {
                            svc::playerLocator.get().has_right_collision = true;
                        }
                    }
                    if(svc::playerLocator.get().physics.mtv.x < 0.1f) {
                        svc::playerLocator.get().has_left_collision = false;
                    }
                    if(svc::playerLocator.get().physics.mtv.x > -0.0f) {
                        svc::playerLocator.get().has_right_collision = false;
                    }
                    
                    svc::playerLocator.get().sync_components();
                    
                    //only for landing
                    if(svc::playerLocator.get().physics.velocity.y > 0.0f) {
                        svc::playerLocator.get().physics.acceleration.y = 0.0f;
                        svc::playerLocator.get().physics.velocity.y = 0.0f;
                    }
                    //player hits the ceiling
                    if(svc::playerLocator.get().physics.velocity.y < -0.01f) {
                        svc::playerLocator.get().physics.acceleration.y = 0.0f;
                        svc::playerLocator.get().physics.velocity.y *= -1;
                        svc::playerLocator.get().physics.mtv.y *= -1;
                        svc::playerLocator.get().jump_hold = false;
                    }
                    svc::playerLocator.get().physics.mtv = {0.0f, 0.0f};
                    svc::playerLocator.get().just_collided = true;
                    svc::playerLocator.get().is_colliding_with_level = true;
                    
                }
                
            }
            if(svc::playerLocator.get().jumpbox.SAT(cell.bounding_box)) {
                if(cell.value > 0) {
                    is_any_jump_colllision = true;
                }
            }
        }
    }
    if(is_any_jump_colllision) {
        svc::playerLocator.get().grounded = true;
    } else {
        svc::playerLocator.get().grounded = false;
    }
    if(is_any_colllision) {
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
