
#pragma once

#include <array>
#include <cstdio>
#include <list>
#include <memory>
#include <unordered_map>
#include "Weapon.hpp"
#include "../setup/ServiceLocator.hpp"

namespace arms {

using Key = WEAPON_TYPE;
constexpr static int max_weapons{20};

struct Arsenal {

	Arsenal() = default;
	Arsenal& operator=(Arsenal&&) = delete;

	void init();
	void push_to_loadout(int id);
	void switch_weapon(float next);
	Weapon& get_current_weapon();
	int get_index();
	void set_index(int index);

	std::array<Weapon, max_weapons> armory{};
	std::vector<Weapon> loadout{};
	std::array<int, max_weapons> extant_projectile_instances{};

  private:
	int current_weapon{};
};

} // namespace arms
