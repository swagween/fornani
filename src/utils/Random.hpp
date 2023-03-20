//
//  Random.hpp
//  fornani
//
//  Created by Alex Frasca on 12/26/22.
//

#pragma once

#include <cstdio>
#include <memory>
#include <random>

namespace util {

static inline auto engine = std::default_random_engine{std::random_device{}()};

class Random {
    
public:
    
    Random() = default;
    
    int random_range(int lo, int hi) {
        return std::uniform_int_distribution<int>{lo, hi}(engine);
    }
    
    float random_range_float(float lo, float hi) {
        return std::uniform_real_distribution<float>{lo, hi}(engine);
    }
    
}; // End Random

}

 /* Random_hpp */
