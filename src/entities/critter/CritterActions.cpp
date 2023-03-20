//
//  CritterActions.cpp
//  components
//
//

#include "Critter.hpp"
#include "../../utils/Random.hpp"

namespace critter {

inline util::Random r{};

void Critter::random_walk() {
    
    bool right{false};
    float rand_dir = r.random_range(0, 100);
    if(rand_dir > 50) {
        right = true;
    }
    
    if(right) {
        physics.acceleration.x = stats.speed;
    } else {
        physics.acceleration.x = -stats.speed;
    }
    
}

} // end components

/* CritterActions_cpp */
