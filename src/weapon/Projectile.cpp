//
//  Projectile.cpp
//  arms
//
//

#include "Projectile.hpp"
#include "../setup/ServiceLocator.hpp"
#include "../setup/LookupTables.hpp"

namespace arms {
    
    Projectile::Projectile() {
        physics = components::PhysicsComponent();
        physics.velocity.x = stats.speed;
        seed();
    };
    Projectile::Projectile(ProjectileStats s, components::PhysicsComponent p, ProjectileAnimation a, WEAPON_TYPE t) : stats(s), physics(p), anim(a), type(t) {
        physics.velocity.x = stats.speed;
        seed();
        set_sprite();
    }

    void Projectile::update() {

        physics.update_euler();
        bounding_box.dimensions = DEFAULT_DIMENSIONS;
        bounding_box.set_position(physics.position);
        position_history.push_back(physics.position);
        if(position_history.size() > history_limit) {
            position_history.pop_front();
        }

        dt = svc::clockLocator.get().tick_rate;

        auto new_time = Clock::now();
        Time frame_time = std::chrono::duration_cast<Time>(new_time - current_time);

        if (frame_time.count() > svc::clockLocator.get().frame_limit) {
            frame_time = Time{ svc::clockLocator.get().frame_limit };
        }
        current_time = new_time;
        accumulator += frame_time;

        int integrations = 0;
        while (accumulator >= dt) {

            stats.lifespan--;

            //animation
            if (curr_frame % anim.framerate == 0) {
                anim_frame++;
            }
            if (anim_frame >= anim.num_frames) { anim_frame = 0; }
            curr_frame++;

            accumulator -= dt;
            ++integrations;
        }
    }

    void Projectile::render(sf::RenderWindow& win, sf::Vector2<float>& campos) {
        if (sp_proj.size() > 0) {
            win.draw(sp_proj.at(0));
        }
        
        //this is the right idea but needs to be refactored and generalized

        float width{};
        for (int i = 0; i < position_history.size(); ++i) {
            switch (i) {
            case 3:
                box.setFillColor(sf::Color{ 243, 239, 255 });
                if (abs(position_history.at(i).x - svc::playerLocator.get().loadout.get_equipped_weapon().barrel_point.x) < 14.0f) {
                    width = abs(position_history.at(i).x - svc::playerLocator.get().loadout.get_equipped_weapon().barrel_point.x);
                }
                else {
                    width = 14.0f;
                }
                box.setSize(sf::Vector2<float>{width, 12.0f});
                    break;
            case 2:
                box.setFillColor(sf::Color{ 236, 201, 255 });
                if (abs(position_history.at(i).x - svc::playerLocator.get().loadout.get_equipped_weapon().barrel_point.x) < 14.0f) {
                    width = abs(position_history.at(i).x - svc::playerLocator.get().loadout.get_equipped_weapon().barrel_point.x);
                }
                else {
                    width = 14.0f;
                }
                box.setSize(sf::Vector2<float>{width, 12.0f});
                    break;
            case 1:
                box.setFillColor(sf::Color{ 206, 170, 255 });
                if (abs(position_history.at(i).x - svc::playerLocator.get().loadout.get_equipped_weapon().barrel_point.x) < 14.0f) {
                    width = abs(position_history.at(i).x - svc::playerLocator.get().loadout.get_equipped_weapon().barrel_point.x);
                }
                else {
                    width = 14.0f;
                }
                box.setSize(sf::Vector2<float>{width, 12.0f});
                    break;
            case 0:
                box.setFillColor(sf::Color{ 164, 133, 255 });
                if(abs(position_history.at(i).x - svc::playerLocator.get().loadout.get_equipped_weapon().barrel_point.x) < 14.0f) {
                    width = abs(position_history.at(i).x - svc::playerLocator.get().loadout.get_equipped_weapon().barrel_point.x);
                } else {
                    width = 14.0f;
                }
                box.setSize(sf::Vector2<float>{width, 12.0f});
                    break;
            }
            box.setPosition(position_history.at(i).x - campos.x, position_history.at(i).y - campos.y - box.getSize().y/2);
            win.draw(box);
        }

        
    }

    void Projectile::destroy() {
        stats.lifespan = -1;
    }

    void Projectile::seed() {
        util::Random r{};
        stats.lifespan += r.random_range(-stats.lifespan_variance, stats.lifespan_variance);
        float var = r.random_range_float(-stats.variance, stats.variance);
        switch (dir) {
        case FIRING_DIRECTION::LEFT:
            physics.velocity = { -stats.speed + (var / 2), var };
            physics.dir = components::DIRECTION::LEFT;
            break;
        case FIRING_DIRECTION::RIGHT:
            physics.velocity = { stats.speed + (var / 2), var };
            physics.dir = components::DIRECTION::RIGHT;
            break;
        case FIRING_DIRECTION::UP:
            physics.velocity = { var, -stats.speed + (var / 2) };
            physics.dir = components::DIRECTION::UP;
            break;
        case FIRING_DIRECTION::DOWN:
            physics.velocity = { var, stats.speed + (var / 2) };
            physics.dir = components::DIRECTION::DOWN;
            break;
        }
    }

    void Projectile::set_sprite() {
        if (anim.num_sprites < 2) { sprite_id = 0; return; }
        util::Random r{};
        sprite_id = r.random_range(0, anim.num_sprites - 1);

        for (int i = 0; i < num_sprites; ++i) {
            sp_proj.push_back(sf::Sprite());
        }

        for(auto& sprite : sp_proj) {
            //sprite.setTexture(lookup::projectile_texture.at(WEAPON_TYPE::BRYNS_GUN));
            sprite.setPosition(physics.position);
        }
        //sp_proj = svc::assetLocator.get().sp_bryns_gun_projectile;
        //sp_proj = lookup::projectile_sprites.at(type);

    }


} // end arms

/* Projectile_cpp */
