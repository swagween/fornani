
#include "Arsenal.hpp"
#include "../service/ServiceProvider.hpp"

namespace arms {

Arsenal::Arsenal(automa::ServiceProvider& svc) : m_services(&svc) {}

void Arsenal::push_to_loadout(int id) {
	if (loadout.empty()) {
		loadout.push_back(std::make_unique<Weapon>(*m_services, m_services->tables.gun_label.at(id), id));
		current_weapon = util::Circuit(static_cast<int>(loadout.size()));
		return;
	}
	loadout.push_back(std::make_unique<Weapon>(*m_services, m_services->tables.gun_label.at(id), id));
	current_weapon = util::Circuit(static_cast<int>(loadout.size()), current_weapon.value().get());
}

void Arsenal::pop_from_loadout(int id) {
	std::erase_if(loadout, [id](auto const& g) { return g->get_id() == id; });
	if (loadout.empty()) {
		current_weapon = {};
		return;
	}
	current_weapon = util::Circuit(static_cast<int>(loadout.size()), current_weapon.value().get());
}

void Arsenal::switch_weapon(automa::ServiceProvider& svc, int next) {
	if (next == 0 || loadout.size() < 2) { return; }
	current_weapon.value().modulate(next);
	svc.soundboard.flags.player.set(audio::Player::arms_switch);
}

std::optional<std::reference_wrapper<std::unique_ptr<Weapon>>> Arsenal::get_weapon_at(int id) {
	if (loadout.empty() || current_weapon.value().get() >= loadout.size()) { return {}; }
	return loadout.at(id);
}

std::optional<std::reference_wrapper<std::unique_ptr<Weapon>>> Arsenal::get_current_weapon() {
	if (loadout.empty() || current_weapon.value().get() >= loadout.size()) { return {}; }
	return loadout.at(current_weapon.value().get());
}

void Arsenal::set_index(int index) { current_weapon.value() = util::Circuit(static_cast<int>(loadout.size()), index); }

bool Arsenal::has(int id) {
	for (auto& gun : loadout) {
		if (gun->get_id() == id) { return true; }
	}
	return false;
}

} // namespace arms
