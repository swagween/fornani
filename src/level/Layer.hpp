//
//  Layer.hpp
//  for_loop
//
//  Created by Alex Frasca on 9/26/20.
//  Copyright © 2020 Western Forest Studios. All rights reserved.
//

#ifndef Layer_hpp
#define Layer_hpp

#include "../utils/Grid.hpp"
#include "Tile.hpp"

class Layer {
    
public:
    
    Layer();
    
    squid::Grid grid;
    uint8_t render_order;
    
};



#endif /* Layer_hpp */
