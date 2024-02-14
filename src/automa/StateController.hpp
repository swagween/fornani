//
//  StateController.hpp
//  automa
//
//  Created by Alex Frasca on 12/26/22.
//

#pragma once

#include <string>

namespace automa {

class StateController {
    
public:

    std::string next_state{};
    bool trigger{ false };
    int source_id{};
    bool save_loaded{ false };
    
}; // End StateController

} // End automa

 /* StateController_hpp */
