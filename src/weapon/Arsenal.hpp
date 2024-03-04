
#pragma once

#include <cstdio>
#include <list>
#include <memory>
#include <unordered_map>
#include <array>
#include "Weapon.hpp"

namespace arms {

using Key = WEAPON_TYPE;
constexpr static int max_weapons{19};

inline Weapon bryns_gun{0};
inline Weapon plasmer{1};
inline Weapon skycorps_ar{2};
inline Weapon tomahawk{3};

struct Arsenal {

	Arsenal() = default;
	Arsenal& operator=(Arsenal&&) = delete;
	
	void init() {
		for (int i = 0; i < max_weapons; ++i) { armory.at(i) = Weapon(i); }
	}
	void push_to_loadout(int id) { loadout.push_back(armory.at(id)); }

	std::array<Weapon, max_weapons> armory{};
	std::vector<Weapon> loadout{};

};

} // namespace arms
