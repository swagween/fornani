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
    
}

void Map::load(const std::string& path) {
    
    std::string filepath = path + "/map_data.txt";
    
    int value{};
    int counter = 0;
    std::ifstream input{};
    input.open(filepath);
    if (!input.is_open()) {
        printf("Failed to open file.\n");
        return;
    }
    
    //dimensions and layers
    input >> value; room_id = value; input.ignore();
    input >> value; dimensions.x = value; input.ignore();
    input >> value; dimensions.y = value; input.ignore();
    input >> value; chunk_dimensions.x = value; input.ignore();
    input >> value; chunk_dimensions.y = value; input.ignore();
    if((dimensions.x / chunk_dimensions.x != CHUNK_SIZE) ||
       (dimensions.y / chunk_dimensions.y != CHUNK_SIZE)) { printf("File is corrupted: Invalid dimensions.\n"); return; }
    real_dimensions = {(float)dimensions.x * CELL_SIZE, (float)dimensions.y * CELL_SIZE};
    for(int i = 0; i < NUM_LAYERS; ++i) {
        layers.push_back(Layer( i, (i == MIDDLEGROUND), dimensions ));
        layers.back().grid.initialize();
    }
    //style
    input >> value; input.ignore();
    if(value >= lookup::get_style.size()) { printf("File is corrupted: Invalid style.\n"); return; } else {
        style = lookup::get_style.at(value);
    }
    //bg;
    input >> value;
    bg = value;
    background = std::make_unique<bg::Background>(bg::bg_behavior_lookup.at(bg), bg);
    input.close();
    
    //get map tiles from text files
    for(auto& layer : layers) {
        input.open(path + "/map_tiles_" + std::to_string(counter) + ".txt");
        for(auto& cell : layer.grid.cells) {
            input >> value;
            squid::TILE_TYPE typ = squid::lookup_type(value);
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
    background->update();
    svc::playerLocator.get().is_any_jump_colllision = false;
    svc::playerLocator.get().is_any_colllision = false;
    svc::playerLocator.get().left_aabb_counter = 0;
    svc::playerLocator.get().right_aabb_counter = 0;
    
    manage_projectiles();
    auto barrier = 3.0f;
    //someday, I will have a for(auto& entity : entities) loop and the player will be included in that
    for(auto& cell : layers.at(MIDDLEGROUND).grid.cells) {
        cell.collision_check = false;
        if(abs(cell.bounding_box.shape_x - svc::playerLocator.get().hurtbox.shape_x) > PLAYER_WIDTH*barrier ||
           abs(cell.bounding_box.shape_y - svc::playerLocator.get().hurtbox.shape_y) > PLAYER_HEIGHT*barrier) {
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
            if(abs(cell.bounding_box.shape_x - proj.bounding_box.shape_x) > PLAYER_WIDTH*barrier ||
               abs(cell.bounding_box.shape_y - proj.bounding_box.shape_y) > PLAYER_HEIGHT*barrier) {
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
                if(abs(cell.bounding_box.shape_x - particle.bounding_box.shape_x) > PLAYER_WIDTH*barrier ||
                   abs(cell.bounding_box.shape_y - particle.bounding_box.shape_y) > PLAYER_HEIGHT*barrier) {
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

void Map::render(sf::RenderWindow& win, std::vector<sf::Sprite>& tileset, sf::Vector2<float> cam) {
    for(auto& proj : active_projectiles) {
        sf::Sprite proj_sprite;
        int curr_frame = proj.sprite_id + proj.anim.num_sprites*proj.anim_frame;
        svc::assetLocator.get().sp_clover_projectile.at(curr_frame).setPosition( {proj.bounding_box.shape_x - cam.x, proj.bounding_box.shape_y - cam.y - proj.bounding_box.shape_h/2} );
        arms::Weapon& curr_weapon = lookup::type_to_weapon.at(proj.type);
        std::vector<sf::Sprite>& curr_proj_sprites = lookup::projectile_sprites.at(curr_weapon.type);
        
        
        if(curr_weapon.type == arms::WEAPON_TYPE::CLOVER) {
            win.draw(svc::assetLocator.get().sp_clover_projectile.at(curr_frame));
        } else if(!curr_proj_sprites.empty()) {
            proj_sprite = curr_proj_sprites.at(arms::ProjDirLookup.at(proj.dir));
            proj_sprite.setPosition({proj.bounding_box.shape_x - cam.x, proj.bounding_box.shape_y - cam.y - proj.bounding_box.shape_h/2} );
            win.draw(proj_sprite);
        }
    }
    
    for(auto& emitter : active_emitters) {
        for(auto& particle : emitter.get_particles()) {
            sf::RectangleShape dot{};
            dot.setFillColor(emitter.color);
            dot.setSize({3.0f, 3.0f});
            dot.setPosition(particle.physics.position.x - cam.x, particle.physics.position.y - cam.y);
            win.draw(dot);
        }
    }
    for(auto& layer : layers) {
        if(layer.render_order >= 4) {
            for(auto& cell : layer.grid.cells) {
                if(cell.value > 0) {
                    int cell_x = cell.bounding_box.shape_x - cam.x;
                    int cell_y = cell.bounding_box.shape_y - cam.y;
                    tileset.at(cell.value).setPosition(cell_x, cell_y);
                    win.draw(tileset.at(cell.value));
                    if(cell.collision_check) {
                        sf::RectangleShape box{};
                        box.setPosition(cell.position.x - cam.x, cell.position.y - cam.y);
                        box.setFillColor(sf::Color{100, 100, 130, 80});
                        box.setOutlineColor(sf::Color(235, 232, 249, 180));
                        box.setOutlineThickness(-2);
                        box.setSize(sf::Vector2<float>{(float)cell.bounding_box.shape_w, (float)cell.bounding_box.shape_h});
//                        win.draw(box);
                    }
                }
            }
        }
    }
}

void Map::render_background(sf::RenderWindow& win, std::vector<sf::Sprite>& tileset, sf::Vector2<float> cam) {
    background->render(win, cam, real_dimensions);
    for(auto& layer : layers) {
        if(layer.render_order < 4) {
            for(auto& cell : layer.grid.cells) {
                if(cell.value > 0) {
                    int cell_x = cell.bounding_box.shape_x - cam.x;
                    int cell_y = cell.bounding_box.shape_y - cam.y;
                    tileset.at(cell.value).setPosition(cell_x, cell_y);
                    win.draw(tileset.at(cell.value));
                }
            }
        }
    }
}

void Map::spawn_projectile_at(sf::Vector2<float> pos) {
    if(active_projectiles.size() < svc::playerLocator.get().loadout.get_equipped_weapon().attributes.rate) {
        active_projectiles.push_back(svc::playerLocator.get().loadout.get_equipped_weapon().projectile);
        active_projectiles.back().set_sprite();
        active_projectiles.back().physics = components::PhysicsComponent({1.0f, 1.0f}, 1.0f);
        active_projectiles.back().physics.position = pos;
        active_projectiles.back().seed();
        active_projectiles.back().update();
        
        active_emitters.push_back(svc::playerLocator.get().loadout.get_equipped_weapon().spray);
        active_emitters.back().get_physics().acceleration = svc::playerLocator.get().physics.acceleration;
        active_emitters.back().set_position(pos.x, pos.y);
        active_emitters.back().set_direction(svc::playerLocator.get().physics.dir);
        active_emitters.back().update();
        
        //temp, I should do this somewhere else
        if(svc::playerLocator.get().loadout.get_equipped_weapon().type == arms::WEAPON_TYPE::PLASMER) {
            svc::assetLocator.get().plasmer_shot.play();
        } else if(svc::playerLocator.get().loadout.get_equipped_weapon().type == arms::WEAPON_TYPE::BRYNS_GUN) {
            svc::assetLocator.get().bg_shot.play();
        } else {
            util::Random r{};
            float randp = r.random_range_float(-0.3, 0.3);
            svc::assetLocator.get().pop_mid.setPitch(1 + randp);
            svc::assetLocator.get().pop_mid.play();
        }
        
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
        spawn_projectile_at(svc::playerLocator.get().get_fire_point());
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
