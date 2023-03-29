//
//  PhysicsComponent.hpp
//  components
//
//

#pragma once

#include "PhysicsComponent.hpp"
#include "../setup/ServiceLocator.hpp"

namespace components {
    
    using Time = std::chrono::duration<float>;
    
    void PhysicsComponent::apply_force(sf::Vector2<float> force) {
        sf::operator+= (acceleration, force);
    }
    
    void PhysicsComponent::apply_force_at_angle(float magnitude, float angle) {
        acceleration.x += (magnitude * cos(angle)) / mass;
        acceleration.y += (magnitude * sin(angle)) / mass;
    }
    
    void PhysicsComponent::update_euler() {

        dt = svc::clockLocator.get().tick_rate;

        auto new_time = Clock::now();
        Time frame_time = std::chrono::duration_cast<Time>(new_time - current_time);
        /*printf("frame_time.count(): %i\n", static_cast<int>(frame_time.count()));*/
        if (frame_time.count() > svc::clockLocator.get().frame_limit) {
            frame_time = Time{ svc::clockLocator.get().frame_limit };
        }
        /*printf("frame count:\n %0.5f", frame_count);*/
        current_time = new_time;
        accumulator += frame_time;
        /*printf("accumulator: %.i\n", accumulator.count());*/
        int integrations = 0;
        while (accumulator >= dt) {

            previous_acceleration = acceleration;
            previous_velocity = velocity;
            previous_position = position;
            integrate(svc::clockLocator.get().tick_constant());

            accumulator -= dt;
            ++integrations;
        }

        /*printf("integrations: %i\n\n", integrations);*/
        //fixme: linear interpolation
        /*const float alpha = accumulator.count() / svc::clockLocator.get().tick_constant();
        acceleration = acceleration * alpha + previous_acceleration * (1.0f - alpha);
        velocity = velocity * alpha + previous_velocity * (1.0f - alpha);
        position = position * alpha + previous_position * (1.0f - alpha);*/


    }

    void PhysicsComponent::integrate(float ndt) {

        acceleration.y += gravity * ndt;
        velocity.x = (velocity.x + (acceleration.x / mass) * ndt) * friction.x;
        velocity.y = (velocity.y + (acceleration.y / mass) * ndt) * friction.y;
        if (velocity.x > maximum_velocity.x) { velocity.x = maximum_velocity.x; }
        if (velocity.x < -maximum_velocity.x) { velocity.x = -maximum_velocity.x; }
        if (velocity.y > maximum_velocity.y) { velocity.y = maximum_velocity.y; }
        if (velocity.y < -maximum_velocity.y) { velocity.y = -maximum_velocity.y; }
        position = position + velocity * ndt;

    }
    
    void PhysicsComponent::update() {
        update_euler();
    }
    
    void PhysicsComponent::update_dampen() {
        acceleration /= svc::clockLocator.get().tick_constant();
        update_euler();
        acceleration = {0.0f, 0.0f};
    }

    void PhysicsComponent::zero() {
        acceleration = { 0.0f, 0.0f };
        velocity = { 0.0f, 0.0f };
    }
    

} // end components

/* PhysicsComponent_hpp */
