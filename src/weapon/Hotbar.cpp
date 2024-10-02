#include "Hotbar.hpp"
#include "Hotbar.hpp"
#include "../service/ServiceProvider.hpp"
#include <algorithm>

namespace arms {

Hotbar::Hotbar(int size) : selection{size} {}

void Hotbar::switch_weapon(automa::ServiceProvider& svc, int next) {
	if (next == 0 || ids.size() == 0) { return; }
	selection.modulate(next);
	svc.soundboard.flags.player.set(audio::Player::arms_switch);
}

void Hotbar::set_selection(int id) {
	for (auto& i : ids) { 
		if (id == i) { return; }
		selection.modulate(1);
	}
}

bool Hotbar::add(int id) {
	if (ids.size() == max_size) { return false; }
	ids.push_back(id);
	selection = util::Circuit{static_cast<int>(ids.size())};
	return true;
}

void Hotbar::remove(int id) {
	std::erase_if(ids, [id](auto const& i) { return i == id; });
	if (ids.empty()) { return; }
	selection = util::Circuit{static_cast<int>(ids.size())};
}

} // namespace arms
