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
#include <chrono>

namespace components {

enum class DIRECTION {
    LEFT,
    RIGHT,
    UP,
    DOWN
};

const sf::Vector2<float> FRICTION_DEFAULT = {0.9f, 0.9f};
const float MASS_DEFAULT = 1.0f;
const float UNIVERSAL_MAX_SPEED = 64.0f;

class PhysicsComponent {
public:

    using Clock = std::chrono::steady_clock;
    using Time = std::chrono::duration<float>;
    
    PhysicsComponent() : friction(FRICTION_DEFAULT), mass(MASS_DEFAULT) {};
    PhysicsComponent(sf::Vector2<float> fric, float ma, sf::Vector2<float> max_vel = sf::Vector2<float>{ UNIVERSAL_MAX_SPEED, UNIVERSAL_MAX_SPEED }, float grav = 0.0f) : friction(fric), mass(ma), maximum_velocity(max_vel), gravity(grav) {};

    //basic physics variables
    sf::Vector2<float> acceleration{0.0f, 0.0f};
    sf::Vector2<float> velocity{ 0.0f, 0.0f };
    sf::Vector2<float> position{ 0.0f, 0.0f };
    sf::Vector2<float> friction{ 0.0f, 0.0f };
    sf::Vector2<float> mtv{ 0.0f, 0.0f };

    sf::Vector2<float> previous_acceleration{ 0.0f, 0.0f };
    sf::Vector2<float> previous_velocity{ 0.0f, 0.0f };
    sf::Vector2<float> previous_position{ 0.0f, 0.0f };
    
    //properties
    float mass{1.0f};
    float gravity{0.0f};
    sf::Vector2<float> maximum_velocity{ UNIVERSAL_MAX_SPEED, UNIVERSAL_MAX_SPEED };
    DIRECTION dir{};
    
    void apply_force(sf::Vector2<float> force);
    void apply_force_at_angle(float magnitude, float angle);
    void update_euler();
    void integrate(float ndt);
    void update();
    void update_dampen();
    void zero();

    //fixed physics time step variables
    Time dt{ 0.001f };
    Clock::time_point current_time = Clock::now();
    Time accumulator{ 0.0f };
};

} // end components

/* PhysicsComponent_hpp */
