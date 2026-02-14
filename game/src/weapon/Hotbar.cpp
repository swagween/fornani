
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/weapon/Hotbar.hpp>

namespace fornani::arms {

Hotbar::Hotbar(int size) : selection{size} {}

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
	m_tags.add(tag.data());
	selection = util::Circuit{static_cast<int>(m_tags.size())};
	previous = -1;
}

void Hotbar::remove(std::string_view tag) {
	m_tags.remove(tag.data());
	if (m_tags.is_empty()) { return; }
	selection = util::Circuit{static_cast<int>(m_tags.size())};
	previous = -1;
}

} // namespace fornani::arms
