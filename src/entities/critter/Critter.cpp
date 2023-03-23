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
    if(metadata.gravity) {
        physics.acceleration.y = 0.3f;
    }
    physics.update_dampen();
    behavior.update();
    bounding_box.update(physics.position.x, physics.position.y, dimensions.x, dimensions.y);
    
    if(abs(physics.velocity.x) > 0.2f && !behavior.restricted()) {
        behavior.run();
    }
    
    if(physics.velocity.x < 0.1 && !behavior.restricted()) {
        behavior.idle();
    }
    
    if(flags.seeking) {
        seek_current_target();
    }
    if(abs(physics.position.x - current_target.x) < 4 && abs(physics.position.y - current_target.y) < 4) { flags.seeking = false; }
}

void Critter::render(sf::RenderWindow &win, sf::Vector2<float> campos) {
    sprite.setPosition(physics.position.x - campos.x + dimensions.x / 2, physics.position.y - campos.y);
    
    //get UV coords
    int u = (int)(behavior.get_frame() / spritesheet_dimensions.y) * sprite_dimensions.x;
    int v = (int)(behavior.get_frame() % spritesheet_dimensions.y) * sprite_dimensions.y;
    sprite.setTextureRect(sf::IntRect({u, v}, {sprite_dimensions.x, sprite_dimensions.y}));
    sprite.setOrigin(sprite_dimensions.x/2, dimensions.y/2);
    //flip the sprite based on the critter's direction
    sf::Vector2<float> right_scale = {1.0f, 1.0f};
    sf::Vector2<float> left_scale = {-1.0f, 1.0f};
    
    hurtbox.setSize(dimensions);
    hurtbox.setPosition(physics.position.x - campos.x, physics.position.y - campos.y);
    hurtbox.setFillColor(sf::Color::Transparent);
    hurtbox.setOutlineColor(flcolor::white);
    hurtbox.setOutlineThickness(-1);
    win.draw(sprite);
//    win.draw(hurtbox);
    
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

void Critter::handle_map_collision(const Shape &cell, bool is_ramp) {
    Shape::Vec mtv = bounding_box.testCollisionGetMTV(bounding_box, cell);
    sf::operator+=(physics.position, mtv);
    physics.acceleration.y *= -1.0f;
    physics.acceleration.x *= -1.0f;
    if(abs(mtv.y) > abs(mtv.x)) {
        physics.velocity.y = 0.0f;
    }
    if(abs(mtv.x) > abs(mtv.y)){
        physics.velocity.x = 0.0f;
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
    physics.position.x = pos.x;
    physics.position.y = pos.y;
    bounding_box.update(pos.x, pos.y, dimensions.x, dimensions.y);
}

void Critter::seek_current_target() {
    sf::Vector2<float> desired = current_target - physics.position;
    desired *= stats.speed;
    sf::Vector2<float> steering = desired - physics.velocity;
//    if(steering.x > 0.1) { steering.x = 0.1; }
//    if(steering.x < -0.1) { steering.x = -0.1; }
    physics.apply_force(steering);
}
                                                  

} // end critter

/* Critter_cpp */
