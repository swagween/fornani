//
//  MapLookups.hpp
//
//  Created by Alex Frasca on 12/26/22.
//

#pragma once


#include <SFML/Graphics.hpp>
#include <string>
#include <unordered_map>

namespace lookup {

    //this should not be inline, but rather loaded from a .txt file generated from existing maps (or the metagrid?)
inline std::unordered_map<int, std::string> get_map_label {

    {101,     "FIRSTWIND_PRISON_01"     },
    {108,     "FIRSTWIND_CLOSET_01"     },
    {104,     "FIRSTWIND_ATRIUM_01"     },
    {103,     "FIRSTWIND_LAB_01"     },
    {102,     "FIRSTWIND_CORRIDOR_01"   },
    {105,     "FIRSTWIND_CORRIDOR_02"   },
    {106,     "FIRSTWIND_CARGO_01"   },
    {107,     "FIRSTWIND_SHAFT_01"   },
    {109,     "FIRSTWIND_SHAFT_02"   },
    {110,     "FIRSTWIND_HOLD_01"   },
    {111,     "FIRSTWIND_ATRIUM_02"   },
    {112,     "FIRSTWIND_HANGAR_01"   },
    {113,     "FIRSTWIND_CORRIDOR_03"   },
    {114,     "FIRSTWIND_CLOSET_02"   },
    {115,     "FIRSTWIND_LAB_01"   },
    {116,     "FIRSTWIND_SHAFT_03"   },
    {117,     "FIRSTWIND_BRIDGE_01"   },
    {118,     "FIRSTWIND_BRIDGE_02"   },
    {119,     "FIRSTWIND_OUTSIDE_01"   },
    {120,     "FIRSTWIND_CLOSET_03"   },
    {121,     "FIRSTWIND_INFIRMARY_01"   },
    {122,     "FIRSTWIND_DECK_01"   },

    {800,     "UNDER_LEDGE_01"       },
    {801,     "UNDER_HOME_01"       },
    {802,     "SHADOW_CORRIDOR_01"      },
    {803,     "SHADOW_CORRIDOR_02"      },
    {804,     "SHADOW_LEDGE_01"         }
};

}

/* LookupTables_hpp */
