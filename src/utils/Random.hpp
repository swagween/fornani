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
    
    int random_range(int lo, int hi) {
        return std::uniform_int_distribution<int>{lo, hi}(engine);
    }
    
    float random_range_float(float lo, float hi) {
        return std::uniform_real_distribution<float>{lo, hi}(engine);
    }
    
    int unsigned_coin_flip() {
        auto result = std::uniform_real_distribution<float>{-1.0f, 1.0f}(engine);
        return (result > 0.0f) ? 0 : 1;
    }
    
    int signed_coin_flip() {
        auto result = std::uniform_real_distribution<float>{-1.0f, 1.0f}(engine);
        return (result > 0.0f) ? -1 : 1;
    }
    
    float random_range_normal(const float mean, const float std_dev) {
        return std::normal_distribution<float>{mean, std_dev}(engine);
    }
    
    bool percent_chance(const float percent) {
        auto result = std::uniform_real_distribution<float>{0.0f, 100.0f}(engine);
        return result < percent;
    }
    
private:
    
    std::default_random_engine engine{std::random_device{}()};
    
}; // End Random

} // End util

 /* Random_hpp */
