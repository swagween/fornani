
#include "fornani/weapon/Arsenal.hpp"
#include "fornani/service/ServiceProvider.hpp"

namespace fornani::arms {

Arsenal::Arsenal(automa::ServiceProvider& svc) : m_services(&svc) {}

void Arsenal::push_to_loadout(std::string_view tag) {
	if (has(tag)) { return; }
	loadout.push_back(std::make_unique<Weapon>(*m_services, tag));
}

void Arsenal::pop_from_loadout(std::string_view tag) {
	if (!has(tag)) { return; }
	if (loadout.size() == 1) {
		loadout.clear();
		return;
	}
	std::erase_if(loadout, [tag](auto const& g) { return g->get_tag() == tag; });
}

void Arsenal::reset() {
	for (auto& gun : loadout) { gun->reset(); }
}

Weapon& Arsenal::get_weapon_at(std::string_view tag) {
	auto index{0};
	auto ctr{0};
	for (auto& gun : loadout) {
		if (gun->get_tag() == tag) { index = ctr; }
		++ctr;
	}
	return *loadout.at(index).get();
}

bool Arsenal::has(std::string_view tag) {
	for (auto& gun : loadout) {
		if (gun->get_tag() == tag) { return true; }
	}
	return false;
}

} // namespace fornani::arms
