//
//  LookupTables.hpp
//
//  Created by Alex Frasca on 12/26/22.
//

#pragma once


#include <SFML/Graphics.hpp>
#include <string>

#include "ServiceLocator.hpp"
#include "../weapon/Weapon.hpp"

namespace lookup {

inline std::unordered_map<arms::WEAPON_TYPE, std::vector<sf::Sprite>&> weapon_sprites {
    {arms::WEAPON_TYPE::BRYNS_GUN,  svc::assetLocator.get().sp_bryns_gun},
    {arms::WEAPON_TYPE::PLASMER,    svc::assetLocator.get().sp_plasmer  },
    {arms::WEAPON_TYPE::WASP,       svc::assetLocator.get().sp_bryns_gun},
    {arms::WEAPON_TYPE::BLIZZARD,   svc::assetLocator.get().sp_bryns_gun},
    {arms::WEAPON_TYPE::BISMUTH,    svc::assetLocator.get().sp_bryns_gun},
    {arms::WEAPON_TYPE::UNDERDOG,   svc::assetLocator.get().sp_bryns_gun},
    {arms::WEAPON_TYPE::ELECTRON,   svc::assetLocator.get().sp_bryns_gun},
    {arms::WEAPON_TYPE::CLOVER,     svc::assetLocator.get().sp_clover   },
    {arms::WEAPON_TYPE::TRITON,     svc::assetLocator.get().sp_bryns_gun},
    {arms::WEAPON_TYPE::WILLET_585, svc::assetLocator.get().sp_bryns_gun},
    {arms::WEAPON_TYPE::QUASAR,     svc::assetLocator.get().sp_bryns_gun},
    {arms::WEAPON_TYPE::NOVA,       svc::assetLocator.get().sp_bryns_gun},
    {arms::WEAPON_TYPE::VENOM,      svc::assetLocator.get().sp_bryns_gun},
    {arms::WEAPON_TYPE::TWIN,       svc::assetLocator.get().sp_bryns_gun},
    {arms::WEAPON_TYPE::CARISE,     svc::assetLocator.get().sp_bryns_gun},
    {arms::WEAPON_TYPE::STINGER,    svc::assetLocator.get().sp_bryns_gun},
    {arms::WEAPON_TYPE::TUSK,       svc::assetLocator.get().sp_bryns_gun},
    {arms::WEAPON_TYPE::TOMAHAWK,   svc::assetLocator.get().sp_bryns_gun}
};

inline std::unordered_map<arms::WEAPON_TYPE, std::vector<sf::Sprite>&> projectile_sprites {
    {arms::WEAPON_TYPE::BRYNS_GUN,  svc::assetLocator.get().sp_bryns_gun_projectile},
    {arms::WEAPON_TYPE::PLASMER,    svc::assetLocator.get().sp_plasmer_projectile  },
    {arms::WEAPON_TYPE::WASP,       svc::assetLocator.get().sp_bryns_gun_projectile},
    {arms::WEAPON_TYPE::BLIZZARD,   svc::assetLocator.get().sp_bryns_gun_projectile},
    {arms::WEAPON_TYPE::BISMUTH,    svc::assetLocator.get().sp_bryns_gun_projectile},
    {arms::WEAPON_TYPE::UNDERDOG,   svc::assetLocator.get().sp_bryns_gun_projectile},
    {arms::WEAPON_TYPE::ELECTRON,   svc::assetLocator.get().sp_bryns_gun_projectile},
    {arms::WEAPON_TYPE::CLOVER,     svc::assetLocator.get().sp_clover_projectile   },
    {arms::WEAPON_TYPE::TRITON,     svc::assetLocator.get().sp_bryns_gun_projectile},
    {arms::WEAPON_TYPE::WILLET_585, svc::assetLocator.get().sp_bryns_gun_projectile},
    {arms::WEAPON_TYPE::QUASAR,     svc::assetLocator.get().sp_bryns_gun_projectile},
    {arms::WEAPON_TYPE::NOVA,       svc::assetLocator.get().sp_bryns_gun_projectile},
    {arms::WEAPON_TYPE::VENOM,      svc::assetLocator.get().sp_bryns_gun_projectile},
    {arms::WEAPON_TYPE::TWIN,       svc::assetLocator.get().sp_bryns_gun_projectile},
    {arms::WEAPON_TYPE::CARISE,     svc::assetLocator.get().sp_bryns_gun_projectile},
    {arms::WEAPON_TYPE::STINGER,    svc::assetLocator.get().sp_bryns_gun_projectile},
    {arms::WEAPON_TYPE::TUSK,       svc::assetLocator.get().sp_bryns_gun_projectile},
    {arms::WEAPON_TYPE::TOMAHAWK,   svc::assetLocator.get().sp_bryns_gun_projectile}
};

inline std::unordered_map<arms::WEAPON_TYPE, arms::Weapon&> type_to_weapon {
    {arms::WEAPON_TYPE::BRYNS_GUN,  arms::bryns_gun     },
    {arms::WEAPON_TYPE::PLASMER,    arms::plasmer       },
    {arms::WEAPON_TYPE::WASP,       arms::wasp          },
    {arms::WEAPON_TYPE::BLIZZARD,   arms::blizzard      },
    {arms::WEAPON_TYPE::BISMUTH,    arms::bismuth       },
    {arms::WEAPON_TYPE::UNDERDOG,   arms::underdog      },
    {arms::WEAPON_TYPE::ELECTRON,   arms::electron      },
    {arms::WEAPON_TYPE::CLOVER,     arms::clover        },
    {arms::WEAPON_TYPE::TRITON,     arms::triton        },
    {arms::WEAPON_TYPE::WILLET_585, arms::willet_585    },
    {arms::WEAPON_TYPE::QUASAR,     arms::quasar        },
    {arms::WEAPON_TYPE::NOVA,       arms::nova          },
    {arms::WEAPON_TYPE::VENOM,      arms::venom         },
    {arms::WEAPON_TYPE::TWIN,       arms::twin          },
    {arms::WEAPON_TYPE::CARISE,     arms::carise        },
    {arms::WEAPON_TYPE::STINGER,    arms::stinger       },
    {arms::WEAPON_TYPE::TUSK,       arms::tusk          },
    {arms::WEAPON_TYPE::TOMAHAWK,   arms::tomahawk      }
};

}

/* LookupTables_hpp */
