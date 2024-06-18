
#include "Arsenal.hpp"
#include "../service/ServiceProvider.hpp"

namespace arms {

Arsenal::Arsenal(automa::ServiceProvider& svc) {
	for (int i = 0; i < max_weapons; ++i) { armory.at(i) = std::make_shared<Weapon>(svc, svc.tables.gun_label.at(i), i); }
	default_gun = std::make_shared<Weapon>(svc, "bryn's gun", 0);
}

void Arsenal::push_to_loadout(int id) { loadout.push_back(armory.at(id)); }

void Arsenal::pop_from_loadout(int id) {
	std::erase_if(loadout, [id](auto const& g) { return g->get_id() == id; });
	current_weapon = 0;
}

void Arsenal::switch_weapon(automa::ServiceProvider& svc, float next) {
	if (next == 0.f) { return; }
	if (loadout.empty()) { return; }
	current_weapon += (int)next;
	if (current_weapon <= -1) { current_weapon = loadout.size() - 1; }
	if (current_weapon >= loadout.size()) { current_weapon = 0; }
	if (current_weapon < loadout.size() && current_weapon < extant_projectile_instances.size()) {
		loadout.at(current_weapon)->active_projectiles = extant_projectile_instances.at(current_weapon);
		svc.soundboard.flags.player.set(audio::Player::arms_switch);
	}
}

Weapon& Arsenal::get_current_weapon() {
	if (current_weapon < loadout.size()) {
		return *loadout.at(current_weapon);
	} else {
		return *default_gun;
	}
}

int Arsenal::get_index() { return current_weapon; }

void Arsenal::set_index(int index) { current_weapon = index; }

bool Arsenal::has(int id) {
	for (auto& gun : loadout) {
		if (gun->get_id() == id) { return true; }
	}
	return false;
}

} // namespace arms
