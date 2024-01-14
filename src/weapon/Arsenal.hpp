//
//  Arsenal.hpp
//  fornani
//
//  Created by Alex Frasca on 12/26/22.
//

#pragma once

#include <cstdio>
#include <memory>
#include <list>
#include "Weapon.hpp"
#include <unordered_map>

namespace arms {

using Key = WEAPON_TYPE;

inline Weapon bryns_gun  =  Weapon("bryn's gun"    , Key::BRYNS_GUN      , bryns_gun_attributes,   bryns_gun_stats,  light_gun_spray, bg_anim, {18, 8});
inline Weapon plasmer    =  Weapon("plasmer"       , Key::PLASMER        , plasmer_attributes,     plasmer_stats   , heavy_gun_spray, bg_anim, {22, 10});
inline Weapon wasp       =  Weapon("wasp"          , Key::WASP           , wasp_attributes,        wasp_stats      );
inline Weapon blizzard   =  Weapon("blizzard"      , Key::BLIZZARD       , blizzard_attributes,    blizzard_stats  );
inline Weapon bismuth    =  Weapon("bismuth"       , Key::BISMUTH        , bismuth_attributes,     bismuth_stats   );
inline Weapon underdog   =  Weapon("underdog"      , Key::UNDERDOG       , underdog_attributes,    underdog_stats  );
inline Weapon electron   =  Weapon("electron"      , Key::ELECTRON       , electron_attributes,    electron_stats  );
inline Weapon clover     =  Weapon("clover"        , Key::CLOVER         , clover_attributes,      clover_stats    , bubble_spray, clover_anim, {18, 10});
inline Weapon triton     =  Weapon("triton"        , Key::TRITON         , triton_attributes,      triton_stats    );
inline Weapon willet_585 =  Weapon("willet 585"    , Key::WILLET_585     , willet_585_attributes,  willet_585_stats);
inline Weapon quasar     =  Weapon("quasar"        , Key::QUASAR         , quasar_attributes,      quasar_stats    );
inline Weapon nova       =  Weapon("nova"          , Key::NOVA           , nova_attributes,        nova_stats      );
inline Weapon venom      =  Weapon("venom"         , Key::VENOM          , venom_attributes,       venom_stats     );
inline Weapon twin       =  Weapon("twin"          , Key::TWIN           , twin_attributes,        twin_stats      );
inline Weapon carise     =  Weapon("carise"        , Key::CARISE         , carise_attributes,      carise_stats    );
inline Weapon stinger    =  Weapon("stinger"       , Key::STINGER        , stinger_attributes,     stinger_stats   );
inline Weapon tusk       =  Weapon("tusk"          , Key::TUSK           , tusk_attributes,        tusk_stats      );
inline Weapon tomahawk   =  Weapon("tomahawk"      , Key::TOMAHAWK       , tomahawk_attributes,    tomahawk_stats  );

struct Armory {
    
    Armory() {
        weapons.insert( {Key::BRYNS_GUN , bryns_gun  } );
        weapons.insert( {Key::PLASMER   , plasmer    } );
        weapons.insert( {Key::WASP      , wasp       } );
        weapons.insert( {Key::BLIZZARD  , blizzard   } );
        weapons.insert( {Key::BISMUTH   , bismuth    } );
        weapons.insert( {Key::UNDERDOG  , underdog   } );
        weapons.insert( {Key::ELECTRON  , electron   } );
        weapons.insert( {Key::CLOVER    , clover     } );
        weapons.insert( {Key::TRITON    , triton     } );
        weapons.insert( {Key::WILLET_585, willet_585 } );
        weapons.insert( {Key::QUASAR    , quasar     } );
        weapons.insert( {Key::NOVA      , nova       } );
        weapons.insert( {Key::VENOM     , venom      } );
        weapons.insert( {Key::TWIN      , twin       } );
        weapons.insert( {Key::CARISE    , carise     } );
        weapons.insert( {Key::STINGER   , stinger    } );
        weapons.insert( {Key::TUSK      , tusk       } );
        weapons.insert( {Key::TOMAHAWK  , tomahawk   } );
    }
    std::unordered_map<Key, Weapon> weapons{};
    
};


class Arsenal {
    
public:
    
    Arsenal() {}
    ~Arsenal() { armory.weapons.clear(); }
    Arsenal& operator=(Arsenal&&) = delete;
    
    Weapon& get_weapon(WEAPON_TYPE type) { return armory.weapons.at(type); }
    
    Weapon& get_equipped_weapon() { return armory.weapons.at(equipped_weapon); }
    
    WEAPON_TYPE equipped_weapon{};
    Armory armory{};
    
}; // end Arsenal

} // end arms

 /* Arsenal_hpp */
