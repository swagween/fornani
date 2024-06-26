
#include "Arsenal.hpp"
#include "../service/ServiceProvider.hpp"
#include <algorithm>

namespace arms {

Arsenal::Arsenal(automa::ServiceProvider& svc) : m_services(&svc) {}

void Arsenal::push_to_loadout(int id) {
	if (loadout.empty()) {
		loadout.push_back(std::make_unique<Weapon>(*m_services, m_services->tables.gun_label.at(id), id));
		current_weapon = util::Circuit(static_cast<int>(loadout.size()));
		return;
	}
	loadout.push_back(std::make_unique<Weapon>(*m_services, m_services->tables.gun_label.at(id), id));
	current_weapon = util::Circuit(static_cast<int>(loadout.size()), current_weapon.get());
}

void Arsenal::pop_from_loadout(int id) {
	if (loadout.size() == 1) {
		loadout.clear();
		return;
	}
	std::erase_if(loadout, [id](auto const& g) { return g->get_id() == id; });
	auto const selection = std::clamp(current_weapon.get(), 0, static_cast<int>(loadout.size() - 1));
    current_weapon = util::Circuit(static_cast<int>(loadout.size()), selection);
}

void Arsenal::switch_weapon(automa::ServiceProvider& svc, int next) {
	if (next == 0 || loadout.size() < 2) { return; }
	current_weapon.modulate(next);
	svc.soundboard.flags.player.set(audio::Player::arms_switch);
}

Weapon& Arsenal::get_weapon_at(int id) { return *loadout.at(id).get(); }

Weapon& Arsenal::get_current_weapon() { return *loadout.at(current_weapon.get()).get(); }

void Arsenal::set_index(int index) { current_weapon = util::Circuit(static_cast<int>(loadout.size()), index); }

bool Arsenal::has(int id) {
	for (auto& gun : loadout) {
		if (gun->get_id() == id) { return true; }
	}
	return false;
}

} // namespace arms
