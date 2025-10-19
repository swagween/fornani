#include "fornani/weapon/Hotbar.hpp"
#include "fornani/service/ServiceProvider.hpp"

#include <ccmath/ext/clamp.hpp>

namespace fornani::arms {

Hotbar::Hotbar(int size) : selection{ccm::ext::clamp(size, 1, 3)} {}

void Hotbar::switch_weapon(automa::ServiceProvider& svc, int next) {
	if (next == 0 || m_tags.size() == 0) { return; }
	previous = selection.get();
	selection.modulate(next);
	svc.soundboard.flags.player.set(audio::Player::arms_switch);
}

void Hotbar::set_selection(std::string_view tag) {
	auto ctr{0};
	for (auto& i : m_tags) {
		if (i == tag) { selection.set(ctr); }
		++ctr;
	}
	previous = -1;
}

void Hotbar::sync() { previous = selection.get(); }

bool Hotbar::has(std::string_view tag) const {
	for (auto& i : m_tags) {
		if (i == tag) { return true; }
	}
	return false;
}

void Hotbar::add(std::string_view tag) {
	if (m_tags.size() == max_size) { m_tags.pop_back(); }
	m_tags.push_back(tag.data());
	selection = util::Circuit{static_cast<int>(m_tags.size())};
	previous = -1;
}

void Hotbar::remove(std::string_view tag) {
	std::erase_if(m_tags, [tag](auto const& i) { return i == tag; });
	if (m_tags.empty()) { return; }
	selection = util::Circuit{static_cast<int>(m_tags.size())};
	previous = -1;
}

} // namespace fornani::arms
