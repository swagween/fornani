
#include <fornani/weapon/Layout.hpp>
#include <algorithm>
#include <ranges>

namespace fornani::arms {

Layout::Layout(dj::Json const& in) {
	std::size_t index{};

	for (auto [i, slot] : std::views::enumerate(in["slots"].as_array())) {
		Slot next{};

		for (auto const& pin : slot["pins"].as_array()) { next.pin_locations.push_back(static_cast<PinLocation>(pin.as<int>())); }

		if (!next.pin_locations.empty()) { next.status = SlotStatus::deactivated; }

		if (auto const& active_pin = slot["active_pin"]) {
			next.active_slot.emplace(static_cast<PinLocation>(active_pin.as<int>()));
			next.status = SlotStatus::activated;
		}

		if (auto const& item = slot["item"]) {
			next.item.emplace(item.as_string());
			next.status = SlotStatus::occupied;
		}

		if (next.status != SlotStatus::unavailable) { next.index.emplace(index++); }

		m_slots[i] = std::move(next);
	}
}

void Layout::serialize(dj::Json& out) const {
	out["slots"] = dj::Json::empty_array();
	for (auto const& slot : m_slots) {
		dj::Json next = dj::Json::empty_object();
		for (auto const& pin : slot.pin_locations) { next["pins"].push_back(static_cast<int>(pin)); }
		if (slot.active_slot) { next["active_pin"] = static_cast<int>(*slot.active_slot); }
		if (slot.item) { next["item"] = *slot.item; }
		out["slots"].push_back(next);
	}
}

bool Layout::place_segment(PinLocation pin, std::size_t where) {
	if (!can_place_segment(pin, where)) { return false; }

	m_slots[where].active_slot.emplace(pin);
	m_slots[where].status = SlotStatus::activated;
	return true;
}

bool Layout::place_item(std::string_view tag, PinLocation pin, std::size_t where) {
	if (!can_place_item(pin, where)) { return false; }

	m_slots[where].item.emplace(tag);
	m_slots[where].status = SlotStatus::occupied;
	return true;
}

void Layout::remove_item(std::size_t const index) {
	if (index >= m_slots.size()) { return; }

	m_slots[index].item.reset();
	m_slots[index].status = SlotStatus::activated;
}

void Layout::remove_fixture(std::size_t const index) {
	if (index >= m_slots.size()) { return; }

	m_slots[index].active_slot.reset();
	m_slots[index].status = SlotStatus::deactivated;
}

auto Layout::can_place_segment(PinLocation pin, std::size_t where) const -> bool {
	if (where >= m_slots.size()) { return false; }

	auto const& slot = m_slots[where];
	return slot.status == SlotStatus::deactivated && std::ranges::find(slot.pin_locations, pin) != slot.pin_locations.end();
}

auto Layout::can_place_item(PinLocation pin, std::size_t where) const -> bool {
	if (where >= m_slots.size()) { return false; }

	auto const& slot = m_slots[where];
	return slot.status == SlotStatus::activated && slot.active_slot == pin;
}

auto Layout::has_item(std::string_view tag) const -> bool {
	return std::ranges::any_of(m_slots, [tag](auto const& slot) { return slot.item && *slot.item == tag; });
}

auto Layout::is_empty() const -> bool {
	return std::ranges::none_of(m_slots, [](auto const& slot) { return slot.item.has_value() || slot.active_slot.has_value(); });
}

auto Layout::get_number_of_slots() const -> int {
	return std::ranges::count_if(m_slots, [](auto const& s) { return s.status != SlotStatus::unavailable; });
}

auto Layout::has_matching_pins(Layout const& other) const -> bool {
	for (std::size_t i = 0; i < layout_size_v; ++i) {
		if (m_slots[i].pin_locations != other.m_slots[i].pin_locations) { return false; }
	}
	return true;
}

std::vector<std::string> Layout::get_items() const { return std::vector<std::string>(); }

} // namespace fornani::arms
