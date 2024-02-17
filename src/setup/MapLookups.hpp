
#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include <unordered_map>

namespace lookup {

    inline std::unordered_map<int, std::string> get_map_label{};

    inline std::unordered_map<int, int> save_point_to_room_id {
        {0, 101}


    };

}
