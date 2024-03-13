
#include "Arsenal.hpp"
#include "../setup/ServiceLocator.hpp"

namespace arms {

void Arsenal::init() {
	for (int i = 0; i < max_weapons; ++i) { armory.at(i) = Weapon(i); }
}

void Arsenal::push_to_loadout(int id) { loadout.push_back(armory.at(id)); }

void Arsenal::switch_weapon(float next) {
	if (next == 0.f) { return; }
	if (loadout.empty()) { return; }
	current_weapon += (int)next;
	if (current_weapon <= -1) { current_weapon = loadout.size() - 1; }
	if (current_weapon >= loadout.size()) { current_weapon = 0; }
	if (current_weapon < loadout.size() && current_weapon < extant_projectile_instances.size()) {
		loadout.at(current_weapon).active_projectiles = extant_projectile_instances.at(current_weapon);
		svc::soundboardLocator.get().flags.player.set(audio::Player::arms_switch);
	}
}

Weapon& Arsenal::get_current_weapon() {
	if (current_weapon < loadout.size()) {
		return loadout.at(current_weapon);
	} else {
		return bryns_gun;
	}
}

int Arsenal::get_index() { return current_weapon; }

void Arsenal::set_index(int index) { current_weapon = index; }

} // namespace arms
