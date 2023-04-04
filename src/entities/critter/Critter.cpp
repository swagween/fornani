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
    alert_range.update(collider.physics.position.x - alert_range.shape_w/2, collider.physics.position.y - alert_range.shape_h / 2, alert_range.shape_w, alert_range.shape_h);
    hostile_range.update(collider.physics.position.x - hostile_range.shape_w / 2, collider.physics.position.y - hostile_range.shape_h / 2, hostile_range.shape_w, hostile_range.shape_h);
    ar.setFillColor(sf::Color{ 80, 80, 20, 60 });
    hr.setFillColor(sf::Color{ 80, 40, 20, 60 });
    ar.setOutlineColor(sf::Color{ 80, 80, 80});
    hr.setOutlineColor(sf::Color{ 80, 80, 80});
    ar.setOutlineThickness(-1);
    hr.setOutlineThickness(-1);
    collider.bounding_box.update(collider.physics.position.x, collider.physics.position.y, dimensions.x, dimensions.y);

    if(flags.asleep) {
        behavior.sleep();
        current_target = collider.physics.position;
    } else if(flags.awakened) {
        behavior.awaken();
        current_target = collider.physics.position;
    } else {
        behavior.idle();
    }
    
    if (collider.is_any_jump_colllision) {
        collider.physics.acceleration.y = 0.0f;
        collider.is_any_jump_colllision = false;
    }

    if (collider.physics.acceleration.x < 0.0f && collider.physics.velocity.x > 0.0f) { behavior.facing_lr = behavior::DIR_LR::LEFT; }
    if (collider.physics.acceleration.x > 0.0f && collider.physics.velocity.x < 0.0f) { behavior.facing_lr = behavior::DIR_LR::RIGHT; }


    if (abs(collider.physics.velocity.x) > 0.02f && behavior.current_state.params.behavior_id == "frdog_idle") {
        behavior.run();
    }
    
    if(flags.seeking) {
        seek_current_target();
    }
    if (collider.is_any_jump_colllision) { collider.physics.acceleration.y = 0.0f; }
    if(abs(collider.physics.position.x - current_target.x) < 16.0f && abs(collider.physics.position.y - current_target.y) < 16.0f) { flags.seeking = false; }

    behavior.current_state.update();

    collider.sync_components();
}

void Critter::render(sf::RenderWindow &win, sf::Vector2<float> campos) {
    sprite.setPosition(collider.physics.position.x - campos.x + dimensions.x / 2, collider.physics.position.y - 8 - campos.y);
    
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
    ar.setPosition(alert_range.shape_x - campos.x, alert_range.shape_y - campos.y);
    hr.setPosition(hostile_range.shape_x - campos.x, hostile_range.shape_y - campos.y);
    hurtbox.setFillColor(sf::Color::Transparent);
    hurtbox.setOutlineColor(flcolor::white);
    hurtbox.setOutlineThickness(-1);
    win.draw(ar);
    win.draw(hr);
    win.draw(sprite);
    //win.draw(hurtbox);
    
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
    if (abs(steering.x) < 0.5) { collider.physics.acceleration.x = 0.0f; return; }
    steering *= 0.08f;
    //don't steer through walls
    if (collider.has_right_collision && steering.x > 0.0f) { return; }
    if (collider.has_left_collision  && steering.x < 0.0f) { return; }
    collider.physics.acceleration.x = steering.x;

}
void Critter::wake_up() {
    flags.asleep = false;
    flags.awakened = true;
    flags.awake = false;
}
void Critter::sleep() {
    flags.asleep = true;
    flags.awakened = false;
    flags.awake = false;
}

void Critter::awake() {
    flags.awake = true;
    flags.awakened = false;
    flags.asleep = false;
}
                                                  

} // end critter

/* Critter_cpp */
