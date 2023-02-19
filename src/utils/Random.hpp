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

class Random {
    
public:
    
    Random() = default;
    
    int random_range(int lo, int hi) {
        static auto engine = std::default_random_engine{std::random_device{}()};
        return std::uniform_int_distribution<int>{lo, hi}(engine);
    }
    
}; // End Random

}

 /* Random_hpp */
