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

    set_sprite();
    /*for (auto& collider : colliders) {
        collider.physics = components::PhysicsComponent(sf::Vector2<float>{0.8f, 0.997f}, 1.0f);
        collider.physics.maximum_velocity = sf::Vector2<float>(stats.speed, stats.speed * 4);
        if (metadata.gravity) { collider.physics.gravity = 0.03f; }
    }*/

    alert_range = shape::Shape({ (float)stats.vision * 1.5f, (float)stats.vision * 1.5f });
    hostile_range = shape::Shape({ (float)stats.vision, (float)stats.vision });

    ar.setSize({ (float)(stats.vision * 1.5), (float)(stats.vision * 1.5) });
    hr.setSize({ (float)stats.vision, (float)stats.vision });

    condition.hp = stats.base_hp;

}

void Critter::update() {

    unique_update();
    behavior.update();
    if (!colliders.empty()) {
        alert_range.set_position(sf::Vector2<float>(colliders.at(0).physics.position.x - alert_range.dimensions.x / 2, colliders.at(0).physics.position.y - alert_range.dimensions.y / 2));
        hostile_range.set_position(sf::Vector2<float>(colliders.at(0).physics.position.x - hostile_range.dimensions.x / 2, colliders.at(0).physics.position.y - hostile_range.dimensions.y / 2));
    }

    ar.setFillColor(sf::Color{ 80, 80, 20, 60 });
    hr.setFillColor(sf::Color{ 80, 40, 20, 60 });
    ar.setOutlineColor(sf::Color{ 180, 180, 180});
    hr.setOutlineColor(sf::Color{ 180, 180, 180});
    ar.setOutlineThickness(-1);
    hr.setOutlineThickness(-1);
    
    if(flags.seeking) {
        seek_current_target();
    }

    if (!colliders.empty()) {
        sprite_position = colliders.at(0).physics.position;
    }

    for (auto& collider : colliders) {
        collider.physics.update_euler();
        collider.sync_components();
        collider.update();
    }

    for(auto& hbx : hurtboxes) {
        hbx.update();
        if(facing_lr == behavior::DIR_LR::LEFT) {
            hbx.set_position({ sprite_position.x + hbx.sprite_offset.x, sprite_position.y + hbx.sprite_offset.y });
        } else {
            hbx.set_position({ sprite_position.x + colliders.at(0).dimensions.x + (sprite_dimensions.x / 2) - hbx.sprite_offset.x, sprite_position.y + hbx.sprite_offset.y });
        }
    }

    //get UV coords
    int u = (int)(behavior.get_frame() / spritesheet_dimensions.y) * sprite_dimensions.x;
    int v = (int)(behavior.get_frame() % spritesheet_dimensions.y) * sprite_dimensions.y;
    sprite.setTextureRect(sf::IntRect({ u, v }, { sprite_dimensions.x, sprite_dimensions.y }));
    sprite.setOrigin(sprite_dimensions.x / 2, dimensions.y / 2);


}

void Critter::render(sf::RenderWindow &win, sf::Vector2<float> campos) {
    if (!colliders.empty()) {
        sprite.setPosition(sprite_position.x - offset.x - campos.x + dimensions.x / 2, sprite_position.y - offset.y + 22 - campos.y);
        drawbox.setSize(dimensions);
    }
    ar.setPosition(alert_range.position.x - campos.x, alert_range.position.y - campos.y);
    hr.setPosition(hostile_range.position.x - campos.x, hostile_range.position.y - campos.y);
    drawbox.setPosition(sprite_position.x - campos.x, sprite_position.y - campos.y);
    drawbox.setSize({ 2.f, 2.f });
    drawbox.setFillColor(flcolor::fucshia);
    drawbox.setOutlineColor(flcolor::fucshia);
    drawbox.setOutlineThickness(-1);
    win.draw(drawbox);
    /*win.draw(ar);
    win.draw(hr);*/
    win.draw(sprite);
    svc::counterLocator.get().at(svc::draw_calls)++;
    for (auto& collider : colliders) {
        collider.render(win, campos);
    }
    for(auto& hbx : hurtboxes) {
        drawbox.setFillColor(sf::Color::Transparent);
        drawbox.setOutlineColor(flcolor::goldenrod);
        drawbox.setOutlineThickness(-1);
        drawbox.setSize(hbx.dimensions);
        drawbox.setPosition(hbx.position.x - campos.x, hbx.position.y - campos.y);
        win.draw(drawbox);
    }
    svc::counterLocator.get().at(svc::draw_calls)++;
    sprite_flip();

    //draw health for debug
    hpbox.setFillColor(sf::Color{0, 228, 185});
    hpbox.setSize(sf::Vector2<float>{1.0f, 4.0f});
    for(int i = 0; i < stats.base_hp; ++i) {
        hpbox.setPosition(sprite.getPosition().x + i, sprite.getPosition().y - 14);
        if (i > condition.hp) { hpbox.setFillColor(sf::Color{29, 118, 112}); }
        //win.draw(hpbox);
        svc::counterLocator.get().at(svc::draw_calls)++;
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

    for (auto& collider : colliders) {
        sprite_position = static_cast<sf::Vector2<float>>(pos);
        collider.physics.position = sprite_position + collider.sprite_offset;
        collider.sync_components();
    }

}

void Critter::seek_current_target() {
    sf::Vector2<float> desired = current_target - colliders.at(0).physics.position;
    desired *= stats.speed;
    sf::Vector2<float> steering = desired - colliders.at(0).physics.velocity;
    if (abs(steering.x) < 0.5) { colliders.at(0).physics.acceleration.x = 0.0f; return; }
    steering *= 0.08f;
    colliders.at(0).physics.acceleration.x = steering.x;

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
