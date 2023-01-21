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

const float FRICTION_DEFAULT = 0.999f;
const float MASS_DEFAULT = 1.0f;

class PhysicsComponent {
public:
    
    using Time = std::chrono::duration<float>;
    
    PhysicsComponent() : friction(FRICTION_DEFAULT), mass(MASS_DEFAULT) {};
    PhysicsComponent(float fric, float ma) : friction(fric), mass(ma) {};

    //basic physics variables
    sf::Vector2<float> acceleration{};
    sf::Vector2<float> velocity{};
    sf::Vector2<float> position{};
    
    //friction force, which is optional
    float friction{};
    float mass{};
    
    void apply_force(sf::Vector2<float> force) {
        sf::operator+= (acceleration, force);
    }
    
    void apply_force_at_angle(float magnitude, float angle) {
        acceleration.x += (magnitude * cos(angle)) / mass;
        acceleration.y += (magnitude * sin(angle)) / mass;
    }
    
    void update_euler(Time dt) {
//        sf::operator*=(acceleration, friction);
//        sf::operator*=(acceleration, (1 + dt.count()));
        sf::operator+=(velocity, acceleration);
        sf::operator*=(velocity, friction);
//        sf::operator*=(velocity, (1 + dt.count()));
        sf::operator+=(position, velocity);
        acceleration = {0.0f, 0.0f};
    }
    
    void update() {
        sf::operator+=(velocity, acceleration);
        sf::operator*=(velocity, friction);
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
