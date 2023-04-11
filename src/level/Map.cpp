//
//  Map.cpp
//  for_loop
//
//  Created by Alex Frasca on 10/6/18.
//  Copyright © 2018 Western Forest Studios. All rights reserved.
//

#include "Map.hpp"
#include "../setup/EnumLookups.hpp"
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
            lookup::TILE_TYPE typ = lookup::tile_lookup.at(value);
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

    //get portal data
    input.open(path + "/map_portals.txt");
    if (input.is_open()) {
        while (!input.eof()) {
            entity::Portal p{};
            input >> p.scaled_dimensions.x; input.ignore();
            input >> p.scaled_dimensions.y; input.ignore();
            input >> value; p.activate_on_contact = (bool)value; input.ignore();
            input >> p.source_map_id; input.ignore();
            input >> p.destination_map_id; input.ignore();
            input >> p.scaled_position.x; input.ignore();
            input >> p.scaled_position.y; input.ignore();
            p.update();
            if (p.dimensions.x != 0) { //only push if one was read, otherwise we reached the end of the file
                portals.push_back(p);
                portals.back().update();
            }
        }
        input.close();
    }
    
    critters.push_back(bestiary.get_critter_at(1));
    critters.back().set_position({404, 494});
    critters.back().collider.physics.zero();

    transition.fade_in = true;
    svc::playerLocator.get().unrestrict_inputs();
    
}

void Map::update() {
    background->update();
    svc::playerLocator.get().collider.is_any_jump_colllision = false;
    svc::playerLocator.get().collider.is_any_colllision = false;
    svc::playerLocator.get().collider.left_aabb_counter = 0;
    svc::playerLocator.get().collider.right_aabb_counter = 0;
    
    manage_projectiles();
    auto barrier = 3.0f;
    //someday, I will have a for(auto& entity : entities) loop and the player will be included in that
    for(auto& cell : layers.at(MIDDLEGROUND).grid.cells) {
        cell.collision_check = false;
        if(abs(cell.bounding_box.shape_x - svc::playerLocator.get().collider.bounding_box.shape_x) > PLAYER_WIDTH*barrier ||
           abs(cell.bounding_box.shape_y - svc::playerLocator.get().collider.bounding_box.shape_y) > PLAYER_HEIGHT*barrier) {
            continue;
        } else {
            cell.collision_check = true;
            
            if(cell.value > 0) {
                svc::playerLocator.get().collider.handle_map_collision(cell.bounding_box, cell.type);
            }
        }
    }

    for (auto& cell : layers.at(MIDDLEGROUND).grid.cells) {
        for (auto& critter : critters) {
            if (abs(cell.bounding_box.shape_x - critter.collider.bounding_box.shape_x) > critter.dimensions.x * barrier ||
                abs(cell.bounding_box.shape_y - critter.collider.bounding_box.shape_y) > critter.dimensions.y * barrier) {
                continue;
            }
            else {
                cell.collision_check = true;
                if (cell.value > 0) {
                    critter.collider.handle_map_collision(cell.bounding_box, cell.type);
                }
            }
        }
    }
    for(auto& cell : layers.at(MIDDLEGROUND).grid.cells) {
        for(auto& proj : active_projectiles) {
            if(abs(cell.bounding_box.shape_x - proj.bounding_box.shape_x) > PLAYER_WIDTH * barrier ||
               abs(cell.bounding_box.shape_y - proj.bounding_box.shape_y) > PLAYER_HEIGHT * barrier) {
                continue;
            } else {
                cell.collision_check = true;
                if(proj.bounding_box.SAT(cell.bounding_box) && cell.value > 0) {
                    if(cell.type == lookup::TILE_TYPE::TILE_BREAKABLE) {
                        --cell.value;
                        if (lookup::tile_lookup.at(cell.value) != lookup::TILE_TYPE::TILE_BREAKABLE) {
                            cell.value = 0;
                            active_emitters.push_back(breakable_debris);
                            active_emitters.back().get_physics().acceleration += proj.physics.acceleration;
                            active_emitters.back().set_position(cell.position.x + CELL_SIZE / 2, cell.position.y + CELL_SIZE / 2);
                            active_emitters.back().set_direction(proj.physics.dir);
                            active_emitters.back().update();
                            svc::assetLocator.get().shatter.play();
                        }
                    }
                    proj.destroy();
                }
            }
        }
    }
    for(auto& cell : layers.at(MIDDLEGROUND).grid.cells) {
        for(auto& emitter : active_emitters) {
            for(auto& particle : emitter.get_particles()) {
                if(abs(cell.bounding_box.shape_x - particle.bounding_box.shape_x) > PLAYER_WIDTH * barrier ||
                   abs(cell.bounding_box.shape_y - particle.bounding_box.shape_y) > PLAYER_HEIGHT * barrier) {
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
    
    for(auto& critter : critters) {
        //critter.random_walk(sf::Vector2<int>(120, 180));
        critter.seek_current_target();
        critter.behavior.facing_lr = (svc::playerLocator.get().collider.physics.position.x < critter.collider.physics.position.x) ? behavior::DIR_LR::RIGHT : behavior::DIR_LR::LEFT;
        critter.update();
        if (svc::playerLocator.get().collider.bounding_box.SAT(critter.hostile_range)) {
            critter.current_target = svc::playerLocator.get().collider.physics.position;
            critter.awake();
        } else if (svc::playerLocator.get().collider.bounding_box.SAT(critter.alert_range)) {
            critter.wake_up();
        } else {
            critter.sleep();
        }
        
        critter.random_idle_action();
        while(!critter.idle_action_queue.empty()) {
            critter.behavior.bark();
            critter.idle_action_queue.pop();
        }
        
    }
    //update player flags
    if(svc::playerLocator.get().collider.left_aabb_counter == 0) {
        svc::playerLocator.get().collider.has_left_collision = false;
    }
    if(svc::playerLocator.get().collider.right_aabb_counter == 0) {
        svc::playerLocator.get().collider.has_right_collision = false;
    }
    if(svc::playerLocator.get().collider.is_any_jump_colllision) {
        svc::playerLocator.get().grounded = true;
    } else {
        svc::playerLocator.get().grounded = false;
    }
    if(svc::playerLocator.get().collider.is_any_colllision) {
        svc::playerLocator.get().collider.is_colliding_with_level = true;
    } else {
        svc::playerLocator.get().collider.is_colliding_with_level = false;
    }

    for (auto& portal : portals) {
        portal.update();
        if (svc::playerLocator.get().inspecting && portal.bounding_box.SAT(svc::playerLocator.get().collider.bounding_box)) {
            portal.activated = true;
            svc::playerLocator.get().restrict_inputs();
        }
        if (portal.activated) {
            transition.fade_out = true;
            if (transition.done) {
                try {
                    svc::stateControllerLocator.get().next_state = lookup::get_map_label.at(portal.destination_map_id);
                }
                catch (std::out_of_range) {
                    svc::stateControllerLocator.get().next_state = lookup::get_map_label.at(room_id);
                }
                svc::stateControllerLocator.get().trigger = true;
                svc::stateControllerLocator.get().source_id = portal.source_map_id;
            }
        }
    }

    //check if player died
    if(!svc::playerLocator.get().state_flags.alive) {
        active_emitters.push_back(player_death);
        active_emitters.back().set_position(svc::playerLocator.get().collider.physics.position.x, svc::playerLocator.get().collider.physics.position.y);
        active_emitters.back().set_direction(components::DIRECTION::NONE);
        active_emitters.back().update();
        svc::assetLocator.get().player_death.play();
        game_over = true;
    }

    if(game_over) {
        transition.fade_out = true;
        svc::playerLocator.get().start_over();
        if (transition.done) {
            svc::stateControllerLocator.get().next_state = lookup::get_map_label.at(101); //temporary. later, we will load the last save
            svc::stateControllerLocator.get().trigger = true;
            svc::playerLocator.get().set_position(sf::Vector2<float>(200.f, 390.f));
        }
    }

    if (svc::clockLocator.get().every_x_frames(1)) {
        transition.update();
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
            dot.setSize({particle.size, particle.size});
            dot.setPosition(particle.physics.position.x - cam.x - particle.size, particle.physics.position.y - cam.y - particle.size);
            win.draw(dot);
        }
    }
    
    for(auto& critter : critters) {
        critter.render(win, cam);
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
                        box.setFillColor(sf::Color{100, 100, 130, 20});
                        box.setOutlineColor(sf::Color(235, 232, 249, 80));
                        box.setOutlineThickness(-1);
                        box.setSize(sf::Vector2<float>{(float)cell.bounding_box.shape_w, (float)cell.bounding_box.shape_h});
                        //win.draw(box);
                    }
                }
            }
        }
    }
    for (auto& portal : portals) {
        portal.render(win, cam);
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
        active_emitters.back().get_physics().acceleration += svc::playerLocator.get().collider.physics.acceleration;
        active_emitters.back().set_position(pos.x, pos.y);
        active_emitters.back().set_direction(svc::playerLocator.get().collider.physics.dir);
        active_emitters.back().update();
        
        //temp, I should do this somewhere else
        if(svc::playerLocator.get().loadout.get_equipped_weapon().type == arms::WEAPON_TYPE::PLASMER) {
            svc::assetLocator.get().plasmer_shot.play();
        } else if(svc::playerLocator.get().loadout.get_equipped_weapon().type == arms::WEAPON_TYPE::BRYNS_GUN) {
            svc::assetLocator.get().bg_shot.play();
        } else {
            util::Random r{};
            float randp = r.random_range_float(-0.3f, 0.3f);
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
    
    std::erase_if(active_projectiles,   [](auto const& p) { return p.stats.lifespan < 0;    });
    std::erase_if(active_emitters, [](auto const& p) { return p.particles.empty();          });
    
    if(svc::playerLocator.get().weapon_fired && !svc::playerLocator.get().start_cooldown) {
        spawn_projectile_at(svc::playerLocator.get().get_fire_point());
    }
}

sf::Vector2<float> Map::get_spawn_position(int portal_source_map_id) {
    for (auto& portal : portals) {
        if (portal.source_map_id == portal_source_map_id) {
            return(portal.position);
        }
    }
    return Vec(300.f, 390.f);
}

squid::Tile& Map::tile_at(const uint8_t i, const uint8_t j) {
    //for checking tile value
    if(i * j < layers.at(MIDDLEGROUND).grid.cells.size()) {
        return layers.at(MIDDLEGROUND).grid.cells.at(i + j * layers.at(MIDDLEGROUND).grid.dimensions.x);
    }
}

Shape& Map::shape_at(const uint8_t i, const uint8_t j) {
    //for testing collision
    if(i * j < layers.at(MIDDLEGROUND).grid.cells.size()) {
        return layers.at(MIDDLEGROUND).grid.cells.at(i + j * layers.at(MIDDLEGROUND).grid.dimensions.x).bounding_box;
    }
}

}// End world
