//
//  Critter.cpp
//  critter
//
//

#include "Critter.hpp"
#include "../../utils/Random.hpp"

namespace critter {

inline util::Random r{};

void Critter::update() {

    collider.physics.update_euler();
    behavior.update();
    collider.bounding_box.update(collider.physics.position.x, collider.physics.position.y, dimensions.x, dimensions.y);

    if(metadata.gravity) {
        collider.physics.gravity = 0.33f;
    }
    if (collider.is_any_jump_colllision) {
        collider.physics.acceleration.y = 0.0f;
    }
    
    if(abs(collider.physics.velocity.x) > 0.2f && !behavior.restricted()) {
        behavior.run();
    }
    
    if(collider.physics.velocity.x < 0.1 && !behavior.restricted()) {
        behavior.idle();
    }
    
    if(flags.seeking) {
        seek_current_target();
    }
    if(abs(collider.physics.position.x - current_target.x) < 16.0f && abs(collider.physics.position.y - current_target.y) < 16.0f) { flags.seeking = false; }

    if (collider.physics.acceleration.x < 0.0f && collider.physics.velocity.x > 0.0f) { behavior.facing_lr = behavior::DIR_LR::LEFT; }
    if (collider.physics.acceleration.x > 0.0f && collider.physics.velocity.x < 0.0f) { behavior.facing_lr = behavior::DIR_LR::RIGHT; }

    collider.sync_components();
}

void Critter::render(sf::RenderWindow &win, sf::Vector2<float> campos) {
    sprite.setPosition(collider.physics.position.x - campos.x + dimensions.x / 2, collider.physics.position.y - campos.y);
    
    //get UV coords
    int u = (int)(behavior.get_frame() / spritesheet_dimensions.y) * sprite_dimensions.x;
    int v = (int)(behavior.get_frame() % spritesheet_dimensions.y) * sprite_dimensions.y;
    sprite.setTextureRect(sf::IntRect({u, v}, {sprite_dimensions.x, sprite_dimensions.y}));
    sprite.setOrigin(sprite_dimensions.x/2, dimensions.y/2);
    //flip the sprite based on the critter's direction
    sf::Vector2<float> right_scale = {1.0f, 1.0f};
    sf::Vector2<float> left_scale = {-1.0f, 1.0f};
    
    hurtbox.setSize(dimensions);
    hurtbox.setPosition(collider.physics.position.x - campos.x, collider.physics.position.y - campos.y);
    hurtbox.setFillColor(sf::Color::Transparent);
    hurtbox.setOutlineColor(flcolor::white);
    hurtbox.setOutlineThickness(-1);
    win.draw(sprite);
    win.draw(hurtbox);
    
    //do this after drawing to avoid 1-frame stuttering
    if(behavior.facing_lr == behavior::DIR_LR::LEFT && sprite.getScale() == right_scale) {
        if(!behavior.restricted()) {
            sprite.scale(-1.0f, 1.0f);
            behavior.turn();
        }
    }
    if(behavior.facing_lr == behavior::DIR_LR::RIGHT && sprite.getScale() == left_scale) {
        if(!behavior.restricted()) {
            sprite.scale(-1.0f, 1.0f);
            behavior.turn();
        }
    }
}

void Critter::set_sprite() {
    try {
        sprite.setTexture(get_critter_texture.at(metadata.id));
    } catch(std::out_of_range) {
        printf("Failed to set sprite for critter.\n");
        return;
    }
}

void Critter::set_position(sf::Vector2<int> pos) {
    collider.physics.position.x = pos.x;
    collider.physics.position.y = pos.y;
    collider.sync_components();
}

void Critter::seek_current_target() {
    sf::Vector2<float> desired = current_target - collider.physics.position;
    desired *= stats.speed;
    sf::Vector2<float> steering = desired - collider.physics.velocity;
//    if(steering.x > 0.1) { steering.x = 0.1; }
//    if(steering.x < -0.1) { steering.x = -0.1; }
    //collider.physics.apply_force(steering);
    collider.physics.acceleration.x = steering.x;
}
                                                  

} // end critter

/* Critter_cpp */
