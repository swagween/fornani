
#include <fornani/weapon/Layout.hpp>
#include <algorithm>
#include <ranges>

namespace fornani::arms {

Layout::Layout(dj::Json const& in) {
	std::size_t index{};
	for (auto [i, slot] : std::views::enumerate(in["slots"].as_array())) {
		Slot next{};
		next.status = SlotStatus::unavailable;
		for (auto const& pin : slot["pins"].as_array()) {
			next.pin_locations.push_back(static_cast<PinLocation>(pin.as<int>()));
			next.status = SlotStatus::deactivated;
		}
		if (auto const& s = slot["active_pin"]) {
			next.active_slot.emplace(static_cast<PinLocation>(s.as<int>()));
			next.status = SlotStatus::activated;
		}
		if (auto const& s = slot["item"]) {
			next.item.emplace(s.as_string());
			next.status = SlotStatus::occupied;
		}
		if (next.status != SlotStatus::unavailable) { next.index.emplace(index++); }
		m_slots[i] = next;
	}
}

void Layout::serialize(dj::Json& out) const {
	for (auto const& slot : m_slots) {
		dj::Json next = dj::Json::empty_object();
		for (auto const& pin : slot.pin_locations) { next["pins"].push_back(static_cast<int>(pin)); }
		if (slot.active_slot) { next["active_pin"] = static_cast<int>(*slot.active_slot); }
		if (slot.item) { next["item"] = *slot.item; }
		out["slots"].push_back(next);
	}
}

bool Layout::place_segment(PinLocation pin, std::size_t where) const { return false; }

bool Layout::place_item(PinLocation pin, std::size_t where) const { return false; }

auto Layout::can_place_segment(PinLocation pin, std::size_t where) const -> bool { return false; }

auto Layout::can_place_item(PinLocation pin, std::size_t where) const -> bool { return false; }

auto Layout::is_empty() const -> bool { return false; }

auto Layout::get_number_of_slots() const -> int {
	return std::ranges::count_if(m_slots, [](auto const& s) { return s.status != SlotStatus::unavailable; });
}

std::vector<std::string> Layout::get_items() const { return std::vector<std::string>(); }

} // namespace fornani::arms
