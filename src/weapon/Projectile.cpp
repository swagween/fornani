//
//  Projectile.cpp
//  arms
//
//

#include "Projectile.hpp"
#include "../setup/ServiceLocator.hpp"

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
    }


} // end arms

/* Projectile_cpp */
