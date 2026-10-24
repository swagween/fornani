
#include <fornani/weapon/Layout.hpp>
#include <ranges>

namespace fornani::arms {

Layout::Layout(dj::Json const& in) {
	for (auto [i, slot] : std::views::enumerate(in.as_array())) {
		Slot next{};
		next.pin_location = static_cast<PinLocation>(slot["pin"].as<int>());
		next.status = static_cast<SlotStatus>(slot["status"].as<int>());
		if (auto const& s = slot["slots"]) { next.item.emplace(s.as_string()); }
		m_slots[i] = next;
	}
}

void Layout::serialize(dj::Json& out) const {
	for (auto const& slot : m_slots) {
		dj::Json next = dj::Json::empty_object();
		next["pin"] = static_cast<int>(slot.pin_location);
		next["status"] = static_cast<int>(slot.status);
		if (slot.item) { next["item"] = *slot.item; }
		out["slots"].push_back(next);
	}
}

std::vector<std::string> Layout::get_items() const { return std::vector<std::string>(); }

} // namespace fornani::arms
