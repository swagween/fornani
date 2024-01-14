//
//  Weapon.cpp
//  arms
//
//

#include "Weapon.hpp"
#include "../setup/ServiceLocator.hpp"

namespace arms {

    Weapon::Weapon(std::string lbl, WEAPON_TYPE weapon_type, const WeaponAttributes& wa, const ProjectileStats& ps, const vfx::ElementBehavior spr, const ProjectileAnimation& pa, sf::Vector2<int> dim ) :
        label(lbl),
        type(weapon_type),
        attributes(wa),
        sprite_dimensions(dim)
    {
        projectile = Projectile(ps, components::PhysicsComponent(), pa, weapon_type);
        spray = vfx::Emitter(spr, burst, spray_color.at(type));
        barrel_point = { sprite_position.x + 18, sprite_position.y + 1 };
    }

    void Weapon::update() {

    }

    void Weapon::render(sf::RenderWindow& win, sf::Vector2<float>& campos) {
        sf::Vector2<float> p_pos = { svc::playerLocator.get().apparent_position.x, svc::playerLocator.get().apparent_position.y + svc::playerLocator.get().sprite_offset.y + 8 };
        sf::Vector2<float> h_pos = svc::playerLocator.get().hand_position;
        sp_gun.setPosition(p_pos.x - campos.x, p_pos.y - campos.y);
        set_position(p_pos);
        set_orientation();

        //fire point debug
        sf::RectangleShape box{};
        box.setPosition(barrel_point.x - campos.x - 1, barrel_point.y - campos.y - 1);
        box.setFillColor(flcolor::fucshia);
        box.setSize(sf::Vector2<float>{2.0f, 2.0f});

        win.draw(sp_gun);
        win.draw(box);
    }

    void Weapon::equip() { equipped = true; }
    void Weapon::unequip() { equipped = false; }
    void Weapon::unlock() { unlocked = true; }
    void Weapon::lock() { unlocked = true; }

    bool Weapon::is_equipped() const { return equipped; }
    bool Weapon::is_unlocked() const { return unlocked; }

    void Weapon::set_position(sf::Vector2<float> pos) {
        sprite_position = pos;
    }

    void Weapon::set_orientation() {

        //flip the sprite based on the player's direction
        sf::Vector2<float> right_scale = { 1.0f, 1.0f };
        sf::Vector2<float> left_scale = { -1.0f, 1.0f };
        //rotate the sprite based on the player's direction
        float neutral_rotation{ 0.0f };
        float up_rotation{ -90.f };
        float down_rotation{ 90.f };
        //start from default
        sp_gun.setRotation(neutral_rotation);
        sp_gun.setScale(right_scale);

        switch (svc::playerLocator.get().behavior.facing_und) {
        case behavior::DIR_UND::UP:
            svc::playerLocator.get().behavior.facing_lr == behavior::DIR_LR::RIGHT ? sp_gun.rotate(-90) : sp_gun.rotate(90);
            barrel_point = { sprite_position.x + attributes.barrel_position.at(1), sprite_position.y - attributes.barrel_position.at(0) };
            fire_dir = FIRING_DIRECTION::UP;
            break;
        case behavior::DIR_UND::NEUTRAL:
            //do nothing
            break;
        case behavior::DIR_UND::DOWN:
            svc::playerLocator.get().behavior.facing_lr == behavior::DIR_LR::RIGHT ? sp_gun.rotate(90) : sp_gun.rotate(-90);
            barrel_point = { sprite_position.x + sprite_dimensions.y - attributes.barrel_position.at(1) - sprite_dimensions.y, sprite_position.y + sprite_dimensions.x };
            fire_dir = FIRING_DIRECTION::DOWN;
            break;
        }
        switch (svc::playerLocator.get().behavior.facing_lr) {
        case behavior::DIR_LR::RIGHT:
            if (svc::playerLocator.get().behavior.facing_und == behavior::DIR_UND::NEUTRAL) {
                barrel_point = { sprite_position.x + attributes.barrel_position.at(0), sprite_position.y + attributes.barrel_position.at(1) };
                fire_dir = FIRING_DIRECTION::RIGHT;
            }
            break;
        case behavior::DIR_LR::LEFT:
            if (svc::playerLocator.get().behavior.facing_und == behavior::DIR_UND::NEUTRAL) { barrel_point = { sprite_position.x - attributes.barrel_position.at(0), sprite_position.y + attributes.barrel_position.at(1) }; 
            fire_dir = FIRING_DIRECTION::LEFT;
            }
            else if (svc::playerLocator.get().behavior.facing_und == behavior::DIR_UND::DOWN) { barrel_point.x += 2.0f * attributes.barrel_position.at(1); }
            else if (svc::playerLocator.get().behavior.facing_und == behavior::DIR_UND::UP) { barrel_point.x -= 2.0f * attributes.barrel_position.at(1); }
            sp_gun.scale(-1.0f, 1.0f);
            break;
        }
        projectile.dir = fire_dir;
    }

} // end arms

/* Weapon_cpp */
