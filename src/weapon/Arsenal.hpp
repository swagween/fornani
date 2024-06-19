
#pragma once

#include <array>
#include <cstdio>
#include <list>
#include <memory>
#include <unordered_map>
#include "Weapon.hpp"

namespace arms {

using Key = WEAPON_TYPE;
constexpr static int max_weapons{8};

struct Arsenal {

	Arsenal() = default;
	Arsenal(automa::ServiceProvider& svc);

	void push_to_loadout(int id);
	void pop_from_loadout(int id);
	void switch_weapon(automa::ServiceProvider& svc, float next);
	Weapon& get_current_weapon();
	int get_index();
	void set_index(int index);
	bool has(int id);

	std::array<std::shared_ptr<Weapon>, max_weapons> armory{};
	std::vector<std::shared_ptr<Weapon>> loadout{};
	std::array<int, max_weapons> extant_projectile_instances{};

  private:
	int current_weapon{};
	std::shared_ptr<Weapon> default_gun{};
};

} // namespace arms
