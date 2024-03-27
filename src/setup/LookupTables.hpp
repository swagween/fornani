
#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include "../weapon/Weapon.hpp"
#include "ServiceLocator.hpp"

namespace lookup {

inline std::unordered_map<arms::WEAPON_TYPE, audio::Weapon> gun_sound{
	{arms::WEAPON_TYPE::BRYNS_GUN, audio::Weapon::bryns_gun},	 {arms::WEAPON_TYPE::PLASMER, audio::Weapon::plasmer},		 {arms::WEAPON_TYPE::WASP, audio::Weapon::bryns_gun},
	{arms::WEAPON_TYPE::BLIZZARD, audio::Weapon::bryns_gun},	 {arms::WEAPON_TYPE::BISMUTH, audio::Weapon::bryns_gun},	 {arms::WEAPON_TYPE::UNDERDOG, audio::Weapon::bryns_gun},
	{arms::WEAPON_TYPE::ELECTRON, audio::Weapon::bryns_gun},	 {arms::WEAPON_TYPE::CLOVER, audio::Weapon::clover},		 {arms::WEAPON_TYPE::TRITON, audio::Weapon::bryns_gun},
	{arms::WEAPON_TYPE::WILLET_585, audio::Weapon::bryns_gun},	 {arms::WEAPON_TYPE::QUASAR, audio::Weapon::bryns_gun},		 {arms::WEAPON_TYPE::NOVA, audio::Weapon::nova},
	{arms::WEAPON_TYPE::VENOM, audio::Weapon::bryns_gun},		 {arms::WEAPON_TYPE::TWIN, audio::Weapon::bryns_gun},		 {arms::WEAPON_TYPE::CARISE, audio::Weapon::bryns_gun},
	{arms::WEAPON_TYPE::STINGER, audio::Weapon::bryns_gun},		 {arms::WEAPON_TYPE::TUSK, audio::Weapon::bryns_gun},		 {arms::WEAPON_TYPE::TOMAHAWK, audio::Weapon::tomahawk},
	{arms::WEAPON_TYPE::GRAPPLING_HOOK, audio::Weapon::plasmer}, {arms::WEAPON_TYPE::SKYCORPS_AR, audio::Weapon::bryns_gun},
};

inline std::unordered_map<arms::WEAPON_TYPE, int> type_to_index{{arms::WEAPON_TYPE::BRYNS_GUN, 0}, {arms::WEAPON_TYPE::PLASMER, 1},	   {arms::WEAPON_TYPE::WASP, 17},		   {arms::WEAPON_TYPE::BLIZZARD, 18},
																{arms::WEAPON_TYPE::BISMUTH, 20},  {arms::WEAPON_TYPE::UNDERDOG, 5},   {arms::WEAPON_TYPE::ELECTRON, 6},	   {arms::WEAPON_TYPE::CLOVER, 7},
																{arms::WEAPON_TYPE::TRITON, 8},	   {arms::WEAPON_TYPE::WILLET_585, 9}, {arms::WEAPON_TYPE::QUASAR, 10},		   {arms::WEAPON_TYPE::NOVA, 11},
																{arms::WEAPON_TYPE::VENOM, 12},	   {arms::WEAPON_TYPE::TWIN, 13},	   {arms::WEAPON_TYPE::CARISE, 14},		   {arms::WEAPON_TYPE::STINGER, 15},
																{arms::WEAPON_TYPE::TUSK, 16},	   {arms::WEAPON_TYPE::TOMAHAWK, 3},   {arms::WEAPON_TYPE::GRAPPLING_HOOK, 4}, {arms::WEAPON_TYPE::SKYCORPS_AR, 2}};

} // namespace lookup
