//
//  Critter.cpp
//  critter
//
//

#include "Critter.hpp"
#include "../../utils/Random.hpp"

namespace critter {

inline util::Random r{};

void Critter::sprite_flip() {
    if (flags.flip) { sprite.scale(-1.0f, 1.0f); flags.flip = false; }
    //flip the sprite based on the critter's direction
    sf::Vector2<float> right_scale = { 1.0f, 1.0f };
    sf::Vector2<float> left_scale = { -1.0f, 1.0f };
    if ((facing_lr == behavior::DIR_LR::LEFT && sprite.getScale() == right_scale) || (facing_lr == behavior::DIR_LR::RIGHT && sprite.getScale() == left_scale)) {
        flags.turning = true;
    }
}

void Critter::init() {
    collider = shape::Collider();
    set_sprite();
    collider.physics = components::PhysicsComponent(sf::Vector2<float>{0.8f, 0.997f}, 1.0f);
    collider.physics.maximum_velocity = sf::Vector2<float>(stats.speed, stats.speed * 4);
    if (metadata.gravity) { collider.physics.gravity = 0.03f; }

    alert_range = shape::Shape({ (float)stats.vision * 1.5f, (float)stats.vision * 1.5f });
    hostile_range = shape::Shape({ (float)stats.vision, (float)stats.vision });

    ar.setSize({ (float)(stats.vision * 1.5), (float)(stats.vision * 1.5) });
    hr.setSize({ (float)stats.vision, (float)stats.vision });

    //behavior = components::CritterBehaviorComponent(metadata.id);

}

void Critter::update() {

    behavior.update();
    alert_range.set_position(sf::Vector2<float>(collider.physics.position.x - alert_range.dimensions.x/2, collider.physics.position.y - alert_range.dimensions.y / 2));
    hostile_range.set_position(sf::Vector2<float>(collider.physics.position.x - hostile_range.dimensions.x / 2, collider.physics.position.y - hostile_range.dimensions.y / 2));
    ar.setFillColor(sf::Color{ 80, 80, 20, 60 });
    hr.setFillColor(sf::Color{ 80, 40, 20, 60 });
    ar.setOutlineColor(sf::Color{ 180, 180, 180});
    hr.setOutlineColor(sf::Color{ 180, 180, 180});
    ar.setOutlineThickness(-1);
    hr.setOutlineThickness(-1);
    
    if(flags.seeking) {
        seek_current_target();
    }

    collider.physics.update_euler();
    collider.sync_components();
    collider.update();

    //get UV coords
    int u = (int)(behavior.get_frame() / spritesheet_dimensions.y) * sprite_dimensions.x;
    int v = (int)(behavior.get_frame() % spritesheet_dimensions.y) * sprite_dimensions.y;
    sprite.setTextureRect(sf::IntRect({ u, v }, { sprite_dimensions.x, sprite_dimensions.y }));
    sprite.setOrigin(sprite_dimensions.x / 2, dimensions.y / 2);

    unique_update();

}

void Critter::render(sf::RenderWindow &win, sf::Vector2<float> campos) {

    sprite.setPosition(collider.physics.position.x - campos.x + dimensions.x / 2, collider.physics.position.y - 8 - campos.y);
    hurtbox.setSize(dimensions);
    hurtbox.setPosition(collider.physics.position.x - campos.x, collider.physics.position.y - campos.y);
    ar.setPosition(alert_range.position.x - campos.x, alert_range.position.y - campos.y);
    hr.setPosition(hostile_range.position.x - campos.x, hostile_range.position.y - campos.y);
    hurtbox.setFillColor(sf::Color::Transparent);
    hurtbox.setOutlineColor(flcolor::white);
    hurtbox.setOutlineThickness(-1);
    win.draw(ar);
    win.draw(hr);
    win.draw(sprite);
    //collider.render(win, campos);
    //win.draw(hurtbox);
    sprite_flip();
    
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
    if (abs(steering.x) < 0.5) { collider.physics.acceleration.x = 0.0f; return; }
    steering *= 0.08f;
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
