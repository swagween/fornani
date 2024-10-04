#include "Hotbar.hpp"
#include "Hotbar.hpp"
#include "../service/ServiceProvider.hpp"
#include <algorithm>

namespace arms {

Hotbar::Hotbar(int size) : selection{std::clamp(size, 1, 3)} {}

void Hotbar::switch_weapon(automa::ServiceProvider& svc, int next) {
	if (next == 0 || ids.size() == 0) { return; }
	selection.modulate(next);
	svc.soundboard.flags.player.set(audio::Player::arms_switch);
}

void Hotbar::set_selection(int id) {
	auto ctr{0};
	for(auto& i : ids) {
		if (i == id) { selection.set(ctr); }
		++ctr;
	}
}

bool Hotbar::has(int id) const {
	for (auto& i : ids) {
		if (i == id) { return true; }
	}
	return false;
}

void Hotbar::add(int id) {
	if (ids.size() == max_size) { ids.pop_back(); }
	ids.push_back(id);
	selection = util::Circuit{static_cast<int>(ids.size())};
}

void Hotbar::remove(int id) {
	std::erase_if(ids, [id](auto const& i) { return i == id; });
	if (ids.empty()) { return; }
	selection = util::Circuit{static_cast<int>(ids.size())};
}

} // namespace arms
