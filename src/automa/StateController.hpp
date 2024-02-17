//
//  StateController.hpp
//  automa
//
//  Created by Alex Frasca on 12/26/22.
//

#pragma once

#include <string>

namespace automa {

    enum class menu_type {
    file_select,
    options,
    settings,
    credits
    };

class StateController {
    
public:

    std::string next_state{};
    bool trigger{ false };
    int source_id{};
    bool save_loaded{ false };

    bool trigger_submenu{ false };
    bool exit_submenu{ false };
    menu_type submenu{};
    
}; // End StateController

} // End automa

 /* StateController_hpp */
