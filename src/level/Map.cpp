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
    
    manage_projectiles();
    
    //someday, I will have a for(auto& entity : entities) loop and the player will be included in that
    for(auto& cell : layers.at(MIDDLEGROUND).grid.cells) {
        cell.collision_check = false;
        if(abs(cell.bounding_box.shape_x - svc::playerLocator.get().hurtbox.shape_x) > PLAYER_WIDTH*1.5f ||
           abs(cell.bounding_box.shape_y - svc::playerLocator.get().hurtbox.shape_y) > PLAYER_HEIGHT*1.5f) {
            continue;
        } else {
            cell.collision_check = true;
            
            if(cell.value > 0) {
                svc::playerLocator.get().handle_map_collision(cell.bounding_box, cell.type == squid::TILE_RAMP);
            }
        }
    }
    for(auto& cell : layers.at(MIDDLEGROUND).grid.cells) {
        for(auto& proj : active_projectiles) {
            if(abs(cell.bounding_box.shape_x - proj.bounding_box.shape_x) > PLAYER_WIDTH*1.5f ||
               abs(cell.bounding_box.shape_y - proj.bounding_box.shape_y) > PLAYER_HEIGHT*1.5f) {
                continue;
            } else {
                cell.collision_check = true;
                if(proj.bounding_box.SAT(cell.bounding_box) && cell.value > 0) {
                    proj.destroy();
                }
            }
        }
    }
    for(auto& cell : layers.at(MIDDLEGROUND).grid.cells) {
        for(auto& emitter : active_emitters) {
            for(auto& particle : emitter.get_particles()) {
                if(abs(cell.bounding_box.shape_x - particle.bounding_box.shape_x) > PLAYER_WIDTH*1.5f ||
                   abs(cell.bounding_box.shape_y - particle.bounding_box.shape_y) > PLAYER_HEIGHT*1.5f) {
                    continue;
                } else {
                    cell.collision_check = true;
                    if(particle.bounding_box.SAT(cell.bounding_box) && cell.value > 0) {
                        Shape::Vec mtv = particle.bounding_box.testCollisionGetMTV(particle.bounding_box, cell.bounding_box);
                        sf::operator+=(particle.physics.position, mtv);
                        particle.physics.acceleration.y *= -1.0f;
                        particle.physics.acceleration.x *= -1.0f;
                        if(abs(mtv.y) > abs(mtv.x)) {
                            particle.physics.velocity.y *= -1.0f;
                        }
                        if(abs(mtv.x) > abs(mtv.y)){
                            particle.physics.velocity.x *= -1.0f;
                        }
                    }
                }
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

void Map::spawn_projectile_at(sf::Vector2<float> pos) {
    if(active_projectiles.size() < svc::playerLocator.get().loadout.get_equipped_weapon().attributes.rate) {
        active_projectiles.push_back(svc::playerLocator.get().loadout.get_equipped_weapon().projectile);
        active_projectiles.back().physics = components::PhysicsComponent({1.0f, 1.0f}, 1.0f);
        active_projectiles.back().physics.position = pos;
        active_projectiles.back().seed();
        active_projectiles.back().update();
        
        active_emitters.push_back(svc::playerLocator.get().loadout.get_equipped_weapon().spray);
        active_emitters.back().get_physics().acceleration = svc::playerLocator.get().physics.acceleration;
        active_emitters.back().set_position(pos.x, pos.y);
        active_emitters.back().set_direction(svc::playerLocator.get().physics.dir);
        active_emitters.back().update();
    }
}

void Map::manage_projectiles() {
    for(auto& proj : active_projectiles) {
        proj.update();
    }
    for(auto& spray : active_emitters) {
        spray.update();
    }
    
    if(!active_projectiles.empty()) {
        for (std::vector<arms::Projectile>::iterator it = active_projectiles.begin(); it != active_projectiles.end();)
        {
            if(it->stats.lifespan < 0) {
                active_projectiles.erase(it);
            } else {
                ++it;
            }
        }
    }
    
    if(!active_emitters.empty()) {
        for (std::vector<vfx::Emitter>::iterator it = active_emitters.begin(); it != active_emitters.end();)
        {
            if(it->get_particles().empty()) {
                active_emitters.erase(it);
            } else {
                ++it;
            }
        }
    }
    
    if(svc::playerLocator.get().weapon_fired && !svc::playerLocator.get().start_cooldown) {
        spawn_projectile_at(svc::playerLocator.get().physics.position);
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
