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

struct Armory {
    using Key = WEAPON_TYPE;
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
    Weapon bryns_gun  =  Weapon("Bryn's Gun"    , Key::BRYNS_GUN      , wal.bryns_gun_attributes,   psl.bryns_gun_stats );
    Weapon plasmer    =  Weapon("Plasmer"       , Key::PLASMER        , wal.plasmer_attributes,     psl.plasmer_stats   );
    Weapon wasp       =  Weapon("Wasp"          , Key::WASP           , wal.wasp_attributes,        psl.wasp_stats      );
    Weapon blizzard   =  Weapon("Blizzard"      , Key::BLIZZARD       , wal.blizzard_attributes,    psl.blizzard_stats  );
    Weapon bismuth    =  Weapon("Bismuth"       , Key::BISMUTH        , wal.bismuth_attributes,     psl.bismuth_stats   );
    Weapon underdog   =  Weapon("Underdog"      , Key::UNDERDOG       , wal.underdog_attributes,    psl.underdog_stats  );
    Weapon electron   =  Weapon("Electron"      , Key::ELECTRON       , wal.electron_attributes,    psl.electron_stats  );
    Weapon clover     =  Weapon("Clover"        , Key::CLOVER         , wal.clover_attributes,      psl.clover_stats    );
    Weapon triton     =  Weapon("Triton"        , Key::TRITON         , wal.triton_attributes,      psl.triton_stats    );
    Weapon willet_585 =  Weapon("Willet 585"    , Key::WILLET_585     , wal.willet_585_attributes,  psl.willet_585_stats);
    Weapon quasar     =  Weapon("Quasar"        , Key::QUASAR         , wal.quasar_attributes,      psl.quasar_stats    );
    Weapon nova       =  Weapon("Nova"          , Key::NOVA           , wal.nova_attributes,        psl.nova_stats      );
    Weapon venom      =  Weapon("Venom"         , Key::VENOM          , wal.venom_attributes,       psl.venom_stats     );
    Weapon twin       =  Weapon("Twin"          , Key::TWIN           , wal.twin_attributes,        psl.twin_stats      );
    Weapon carise     =  Weapon("Carise"        , Key::CARISE         , wal.carise_attributes,      psl.carise_stats    );
    Weapon stinger    =  Weapon("Stinger"       , Key::STINGER        , wal.stinger_attributes,     psl.stinger_stats   );
    Weapon tusk       =  Weapon("Tusk"          , Key::TUSK           , wal.tusk_attributes,        psl.tusk_stats      );
    Weapon tomahawk   =  Weapon("Tomahawk"      , Key::TOMAHAWK       , wal.tomahawk_attributes,    psl.tomahawk_stats  );
};
    
static const Armory armory{};

class Arsenal {
    
public:
    
    Arsenal() = default;
    ~Arsenal() { weapons.clear(); }
    Arsenal& operator=(Arsenal&&) = delete;
    
    void add_weapon(WEAPON_TYPE type) {
        std::unordered_map<Armory::Key, Weapon>::const_iterator index = armory.weapons.find(type);
        weapons.push_back(armory.weapons.at(index));
    }
    
    void remove_weapon(WEAPON_TYPE id) {
        
    }
    
    std::list<Weapon> weapons;
    
}; // end Arsenal

} // end arms

 /* Arsenal_hpp */
