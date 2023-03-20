//
//  Clock.hpp
//  fornani
//
//  Created by Alex Frasca on 12/26/22.
//

#pragma once

#include <memory>
#include <random>
#include <chrono>

namespace util {

class Clock {
    
public:
    
    using Clk = std::chrono::steady_clock;
    using Tpt = std::chrono::time_point<std::chrono::steady_clock>;
    using Time = std::chrono::duration<float>;
    
    Clock() = default;
    
    void tick() {
        auto new_time = Clk::now();
        auto frame_time = Time{new_time - current_time};
        current_time = new_time;
        elapsed_time += frame_time;
        seconds += elapsed_time.count();
    }
    
    bool every_x_seconds(int num_seconds) {
        return((int)std::floor(seconds) % num_seconds == 0);
    }
    
    Time elapsed_time{};
    Tpt current_time = Clk::now();
    float seconds{0.0f};
    
}; // End Clock

}

 /* Clock_hpp */
