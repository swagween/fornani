//
//  PhysicsComponent.hpp
//  components
//
//

#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>
#include <random>

namespace components {

const sf::Vector2<float> FRICTION_DEFAULT = {0.9f, 0.9f};
const float MASS_DEFAULT = 1.0f;

class PhysicsComponent {
public:
    
    using Time = std::chrono::duration<float>;
    
    PhysicsComponent() : friction(FRICTION_DEFAULT), mass(MASS_DEFAULT) {};
    PhysicsComponent(sf::Vector2<float> fric, float ma) : friction(fric), mass(ma) {};

    //basic physics variables
    sf::Vector2<float> acceleration{};
    sf::Vector2<float> velocity{};
    sf::Vector2<float> position{};
    sf::Vector2<float> friction{};
    sf::Vector2<float> mtv{};
    
    float mass{};
    
    void apply_force(sf::Vector2<float> force) {
        sf::operator+= (acceleration, force);
    }
    
    void apply_force_at_angle(float magnitude, float angle) {
        acceleration.x += (magnitude * cos(angle)) / mass;
        acceleration.y += (magnitude * sin(angle)) / mass;
    }
    
    void update_euler(Time dt) {
        sf::operator+=(velocity, acceleration);
        velocity.x *= friction.x;
        velocity.y *= friction.y;
        if(abs(mtv.y) < 0.0001) {
            sf::operator+=(position, velocity);
        } else {
            position.y += mtv.y;
        }
    }
    
    void update() {
        sf::operator+=(velocity, acceleration);
        velocity.x *= friction.x;
        velocity.y *= friction.y;
        sf::operator+=(position, velocity);
        acceleration = {0.0f, 0.0f};
    }
    
    int random_range(int lo, int hi) {
        static auto engine = std::default_random_engine{std::random_device{}()};
        return std::uniform_int_distribution<int>{lo, hi}(engine);
    }
    
};

} // end components

/* PhysicsComponent_hpp */
