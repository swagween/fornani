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

    //get inspectable data
    input.open(path + "/map_inspectables.txt");
    if (input.is_open()) {
        while (!input.eof()) {
            entity::Inspectable p{};
            input >> p.scaled_dimensions.x; input.ignore();
            input >> p.scaled_dimensions.y; input.ignore();
            input >> value; p.activate_on_contact = (bool)value; input.ignore(); input.ignore();
            std::getline(input, p.message, '#');
            input >> p.scaled_position.x; input.ignore();
            input >> p.scaled_position.y; input.ignore();
            p.update();
            if (p.dimensions.x != 0) { //only push if one was read, otherwise we reached the end of the file
                inspectables.push_back(p);
                inspectables.back().update();
            }
        }
        input.close();
    }

    //get animator data
    input.open(path + "/map_animators.txt");
    if (input.is_open()) {
        while (!input.eof()) {
            entity::Animator p{};
            input >> p.scaled_dimensions.x; input.ignore();
            input >> p.scaled_dimensions.y; input.ignore();
            input >> value; p.id = value; input.ignore();
            input >> value; p.automatic = (bool)value; input.ignore();
            input >> value; p.foreground = (bool)value; input.ignore();
            input >> p.scaled_position.x; input.ignore();
            input >> p.scaled_position.y; input.ignore();
            if (p.scaled_dimensions.x != 0) { //only push if one was read, otherwise we reached the end of the file
                uint32_t large_dim = 16;
                p.dimensions = static_cast<Vec>(p.scaled_dimensions * large_dim);
                p.bounding_box = shape::Shape(p.dimensions);
                animators.push_back(p);
            }
        }
        input.close();
    }

    //get critter data
    //zero the pool_counter
    critter::pool_counter.fill(0);
    input.open(path + "/map_critters.txt");
    if (input.is_open()) {
        while (!input.eof()) {

            int id{}; sf::Vector2<int> pos{};

            //extract id and position
            input >> id; input.ignore();
            input >> pos.x; input.ignore();
            input >> pos.y; input.ignore();

            //fetch the type
            critter::CRITTER_TYPE type = critter::get_critter_type.at(id);

            //push the critter
            //which type of critter? and how deep into the pool are we?
            critters.push_back(*bestiary.fetch_critter_of_type(type, critter::pool_counter.at(id)));
            critters.back()->set_position({ pos.x * TILE_WIDTH, pos.y * TILE_WIDTH });
            critters.back()->collider.physics.zero();
            critter::pool_counter.at(id)++;

        }
        input.close();
    }

    for(auto& critter : critters) {

        colliders.push_back(&critter->collider);
    
    }
    colliders.push_back(&svc::playerLocator.get().collider);
    for(auto& a : svc::playerLocator.get().antennae) {
        colliders.push_back(&a.collider);
    }

    transition.fade_in = true;
    svc::playerLocator.get().unrestrict_inputs();
    minimap = sf::View(sf::FloatRect(0.0f, 0.0f, cam::screen_dimensions.x * 2, cam::screen_dimensions.y * 2));
    minimap.setViewport(sf::FloatRect(0.0f, 0.75f, 0.2f, 0.2f));
    
}

void Map::update() {

    svc::consoleLocator.get().update();

    background->update();
    svc::playerLocator.get().collider.reset();
    for (auto& a : svc::playerLocator.get().antennae) {
        a.collider.reset();
    }

    for (auto& critter : critters) {
        critter->collider.reset();
    }

    manage_projectiles();
    auto barrier = 3.0f;

    //someday, I will have a for(auto& entity : entities) loop and the player will be included in that

    for(auto& collider : colliders) {
        for (auto& cell : layers.at(MIDDLEGROUND).grid.cells) {
            cell.collision_check = false;
            if (abs(cell.bounding_box.position.x - collider->bounding_box.position.x) > lookup::unit_size_i * barrier ||
                abs(cell.bounding_box.position.y - collider->bounding_box.position.y) > lookup::unit_size_i * (barrier - 1)) {
                continue;
            }
            else {
                cell.collision_check = true;
                if (cell.value > 0) {
                    collider->handle_map_collision(cell.bounding_box, cell.type);
                }
            }
            
        }
    }

    for(auto& cell : layers.at(MIDDLEGROUND).grid.cells) {
        for(auto& proj : active_projectiles) {
            if(abs(cell.bounding_box.position.x - proj.bounding_box.position.x) > lookup::unit_size_i * barrier ||
               abs(cell.bounding_box.position.y - proj.bounding_box.position.y) > lookup::unit_size_i * barrier) {
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
                    proj.destroy(false);
                }
            }
        }
    }

    for(auto& cell : layers.at(MIDDLEGROUND).grid.cells) {
        for(auto& emitter : active_emitters) {
            for(auto& particle : emitter.get_particles()) {
                if(abs(cell.bounding_box.position.x - particle.bounding_box.position.x) > lookup::unit_size_i * barrier ||
                   abs(cell.bounding_box.position.y - particle.bounding_box.position.y) > lookup::unit_size_i * barrier) {
                    continue;
                } else {
                    cell.collision_check = true;
                    if(particle.bounding_box.SAT(cell.bounding_box) && cell.value > 0) {
                        shape::Shape::Vec mtv = particle.bounding_box.testCollisionGetMTV(particle.bounding_box, cell.bounding_box);
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

    for (auto& proj : active_projectiles) {
        if (proj.state.test(arms::ProjectileState::destruction_initiated)) { continue; }
        for (auto& critter : critters) {
            if (proj.bounding_box.SAT(critter->collider.bounding_box)) {
                critter->flags.shot = true;
                if (critter->flags.vulnerable) {
                    critter->flags.hurt = true;
                    critter->flags.just_hurt = true;
                    critter->condition.hp -= proj.stats.damage;
                }
                proj.destroy(false);
            }
        }
    }
    
    for(auto& critter : critters) {

        critter->facing_lr = (svc::playerLocator.get().collider.physics.position.x < critter->collider.physics.position.x) ? behavior::DIR_LR::RIGHT : behavior::DIR_LR::LEFT;
        //critter->random_walk(sf::Vector2<int>(120, 180));
        if (svc::playerLocator.get().collider.bounding_box.SAT(critter->hostile_range)) {
            critter->current_target = svc::playerLocator.get().collider.physics.position;
            critter->awake();
        } else if (svc::playerLocator.get().collider.bounding_box.SAT(critter->alert_range)) {
            critter->wake_up();
        } else {
            critter->sleep();
        }

    }

    for(auto& collider : colliders) {
        collider->reset_ground_flags();
    }


    for (auto& portal : portals) {
        portal.update();
        portal.handle_activation(room_id, transition.fade_out, transition.done);
    }

    for (auto& inspectable : inspectables) {
        if (svc::playerLocator.get().flags.input.test(Input::inspecting) && inspectable.bounding_box.SAT(svc::playerLocator.get().collider.bounding_box)) {
            inspectable.activated = true;
            svc::consoleLocator.get().flags.set(gui::ConsoleFlags::active);
            svc::playerLocator.get().restrict_inputs();
        }
        if(inspectable.activated && svc::consoleLocator.get().flags.test(gui::ConsoleFlags::active)) {
            svc::consoleLocator.get().begin();
            if(svc::playerLocator.get().flags.input.test(Input::exit_request)) {
                inspectable.activated = false;
                svc::consoleLocator.get().end();
            }
        }
    }

    for (auto& animator : animators) {
        if (animator.bounding_box.SAT(svc::playerLocator.get().collider.bounding_box) && svc::playerLocator.get().moving_at_all()) {
            animator.anim.on();
            animator.activated = true;
        } else {
            animator.activated = false;
        }
        animator.update();
    }

    //check if player died
    if(!svc::playerLocator.get().flags.state.test(State::alive) && !game_over) {
        active_emitters.push_back(player_death);
        active_emitters.back().get_physics().acceleration += svc::playerLocator.get().collider.physics.acceleration;
        active_emitters.back().set_position(svc::playerLocator.get().collider.physics.position.x, svc::playerLocator.get().collider.physics.position.y);
        active_emitters.back().set_direction(components::DIRECTION::DOWN);
        active_emitters.back().update();
        svc::assetLocator.get().player_death.play();
        game_over = true;
    }

    if(game_over) {
        transition.fade_out = true;
        if (transition.done) {
            svc::playerLocator.get().start_over();
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
        proj.render(win, cam);
        /*sf::Sprite proj_sprite;
        int curr_frame = proj.sprite_id + proj.anim.num_sprites*proj.anim_frame;
        svc::assetLocator.get().sp_clover_projectile.at(curr_frame).setPosition( {proj.bounding_box.position.x - cam.x, proj.bounding_box.position.y - cam.y - proj.bounding_box.dimensions.y/2} );
        arms::Weapon& curr_weapon = lookup::type_to_weapon.at(proj.type);
        std::vector<sf::Sprite>& curr_proj_sprites = lookup::projectile_sprites.at(curr_weapon.type);
        
        
        if(curr_weapon.type == arms::WEAPON_TYPE::CLOVER) {
            win.draw(svc::assetLocator.get().sp_clover_projectile.at(curr_frame));
        } else if(!curr_proj_sprites.empty()) {
            proj_sprite = curr_proj_sprites.at(arms::ProjDirLookup.at(proj.dir));
            proj_sprite.setPosition({proj.bounding_box.position.x - cam.x, proj.bounding_box.position.y - cam.y - proj.bounding_box.dimensions.y/2} );
            win.draw(proj_sprite);
        }*/
    }
    
    //emitters
	for (auto& emitter : active_emitters) {
		for (auto& particle : emitter.get_particles()) {
			sf::RectangleShape dot{};
			dot.setFillColor(emitter.color);
			dot.setSize({ particle.size, particle.size });
			dot.setPosition(particle.physics.position.x - cam.x, particle.physics.position.y - cam.y);
			win.draw(dot);
            svc::counterLocator.get().at(svc::draw_calls)++;
		}
	}

    //player
    sf::Vector2<float> player_pos = svc::playerLocator.get().apparent_position - svc::cameraLocator.get().physics.position;
    svc::playerLocator.get().render(win, svc::cameraLocator.get().physics.position);
    
    //enemies
    for(auto& critter : critters) {
        critter->render(win, cam);
    }

    //foreground animators
    for (auto& animator : animators) {
        if (!animator.foreground) { animator.render(win, cam); }
    }
    
    //level foreground
    for(auto& layer : layers) {
        if(layer.render_order >= 4) {
            for(auto& cell : layer.grid.cells) {
                if(cell.value > 0) {
                    int cell_x = cell.bounding_box.position.x - cam.x;
                    int cell_y = cell.bounding_box.position.y - cam.y;
                    tileset.at(cell.value).setPosition(cell_x, cell_y);
                    if (!svc::globalBitFlagsLocator.get().test(svc::global_flags::greyblock_state) || layer.render_order == 4) {
                        if (svc::cameraLocator.get().within_frame(cell_x + CELL_SIZE, cell_y + CELL_SIZE)) {
                            win.draw(tileset.at(cell.value));
                            svc::counterLocator.get().at(svc::draw_calls)++;
                        }
                    }
                    if(cell.collision_check && svc::globalBitFlagsLocator.get().test(svc::global_flags::greyblock_state)) {
                        sf::RectangleShape box{};
                        box.setPosition(cell.bounding_box.vertices[0].x - cam.x, cell.bounding_box.vertices[0].y - cam.y);
                        box.setFillColor(sf::Color{100, 100, 130, 80});
                        box.setOutlineColor(sf::Color(235, 232, 249, 140));
                        box.setOutlineThickness(-1);
                        box.setSize(sf::Vector2<float>{(float)cell.bounding_box.dimensions.x, (float)cell.bounding_box.dimensions.y});
                        win.draw(box);
                        svc::counterLocator.get().at(svc::draw_calls)++;
                    }
                }
            }
        }
    }

    if (real_dimensions.y < cam::screen_dimensions.y) {
        float ydiff = (cam::screen_dimensions.y - real_dimensions.y) / 2;
        borderbox.setFillColor(flcolor::black);
        borderbox.setSize({ (float)cam::screen_dimensions.x, ydiff });
        borderbox.setPosition(0.0f, 0.0f);
        win.draw(borderbox);
        svc::counterLocator.get().at(svc::draw_calls)++;
        borderbox.setPosition(0.0f, real_dimensions.y + ydiff);
        win.draw(borderbox);
        svc::counterLocator.get().at(svc::draw_calls)++;
    }
    if (real_dimensions.x < cam::screen_dimensions.x) {
        float xdiff = (cam::screen_dimensions.x - real_dimensions.x) / 2;
        borderbox.setFillColor(flcolor::black);
        borderbox.setSize({ xdiff, (float)cam::screen_dimensions.y });
        borderbox.setPosition(0.0f, 0.0f);
        win.draw(borderbox);
        svc::counterLocator.get().at(svc::draw_calls)++;
        borderbox.setPosition(real_dimensions.x + xdiff, 0.0f);
        win.draw(borderbox);
        svc::counterLocator.get().at(svc::draw_calls)++;
    }

    for (auto& portal : portals) {
        portal.render(win, cam);
    }
    for (auto& inspectable : inspectables) {
        inspectable.render(win, cam); //for debug
    }
    for(auto& animator : animators) {
        if (animator.foreground) { animator.render(win, cam); }
    }

    //render minimap
    if (show_minimap) {
        win.setView(minimap);
        for (auto& cell : layers.at(MIDDLEGROUND).grid.cells) {
            minimap_tile.setPosition(cell.position.x - cam.x, cell.position.y - cam.y);
            minimap_tile.setSize(sf::Vector2<float>{(float)cell.bounding_box.dimensions.x, (float)cell.bounding_box.dimensions.y});
            if (cell.value > 0) {
                minimap_tile.setFillColor(sf::Color{20, 240, 20, 120});
                win.draw(minimap_tile);
                svc::counterLocator.get().at(svc::draw_calls)++;
            }
            else {
                minimap_tile.setFillColor(sf::Color{ 20, 20, 20, 120 });
                win.draw(minimap_tile);
                svc::counterLocator.get().at(svc::draw_calls)++;
            }
        }
        minimap_tile.setPosition(svc::playerLocator.get().collider.physics.position.x - cam.x, svc::playerLocator.get().collider.physics.position.y - cam.y);
        minimap_tile.setFillColor(sf::Color{ 240, 240, 240, 180 });
        win.draw(minimap_tile);
        svc::counterLocator.get().at(svc::draw_calls)++;
        win.setView(sf::View(sf::FloatRect{ 0.f, 0.f, (float)cam::screen_dimensions.x, (float)cam::screen_dimensions.y }));
    }

}

void Map::render_background(sf::RenderWindow& win, std::vector<sf::Sprite>& tileset, sf::Vector2<float> cam) {
    if (!svc::globalBitFlagsLocator.get().test(svc::global_flags::greyblock_state)) {
        background->render(win, cam, real_dimensions);
    }else {
        sf::RectangleShape box{};
        box.setPosition(0, 0);
        box.setFillColor(flcolor::black);
        box.setSize({ (float)cam::screen_dimensions.x, (float)cam::screen_dimensions.y });
        win.draw(box);
        svc::counterLocator.get().at(svc::draw_calls)++;
        }
    if (real_dimensions.y < cam::screen_dimensions.y) { svc::cameraLocator.get().fix_horizontally(real_dimensions); }
    for(auto& layer : layers) {
        if(layer.render_order < 4) {
            for(auto& cell : layer.grid.cells) {
                if(cell.value > 0) {
                    int cell_x = cell.bounding_box.position.x - cam.x;
                    int cell_y = cell.bounding_box.position.y - cam.y;
                    tileset.at(cell.value).setPosition(cell_x, cell_y);
                    if (!svc::globalBitFlagsLocator.get().test(svc::global_flags::greyblock_state)) {
                        if (svc::cameraLocator.get().within_frame(cell_x + CELL_SIZE, cell_y + CELL_SIZE)) {
                            win.draw(tileset.at(cell.value));
                            svc::counterLocator.get().at(svc::draw_calls)++;
                        }
                    }
                }
            }
        }
    }
}

void Map::render_console(sf::RenderWindow& win) {
    if (svc::consoleLocator.get().flags.test(gui::ConsoleFlags::active)) {
        svc::consoleLocator.get().render(win);
        for (auto& inspectable : inspectables) {
            if (inspectable.activated) {
                svc::consoleLocator.get().write(win, inspectable.message);
                //svc::consoleLocator.get().write(win, "ab?:-_()#`");
            }
        }
    }
}

void Map::spawn_projectile_at(sf::Vector2<float> pos) {
    if(active_projectiles.size() < svc::playerLocator.get().loadout.get_equipped_weapon().attributes.rate) {
        active_projectiles.push_back(svc::playerLocator.get().loadout.get_equipped_weapon().projectile);
        active_projectiles.back().fired_point = svc::playerLocator.get().loadout.get_equipped_weapon().barrel_point;
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
    
    std::erase_if(active_projectiles,   [](auto const& p) { return p.state.test(arms::ProjectileState::destroyed);    });
    std::erase_if(active_emitters, [](auto const& p) { return p.particles.empty();          });
    std::erase_if(critters, [](auto const& c) { return c->condition.hp <= 0;                 });
    
    if (!svc::playerLocator.get().weapons_hotbar.empty()) {
        if (svc::playerLocator.get().weapon_fired && !svc::playerLocator.get().start_cooldown) {
            spawn_projectile_at(svc::playerLocator.get().loadout.get_equipped_weapon().barrel_point);
        }
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

shape::Shape& Map::shape_at(const uint8_t i, const uint8_t j) {
    //for testing collision
    if(i * j < layers.at(MIDDLEGROUND).grid.cells.size()) {
        return layers.at(MIDDLEGROUND).grid.cells.at(i + j * layers.at(MIDDLEGROUND).grid.dimensions.x).bounding_box;
    }
}

}// End world
