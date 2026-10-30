
#pragma once

#include <djson/json.hpp>
#include <array>
#include <optional>

namespace fornani::arms {

constexpr std::size_t layout_size_v = 5;

enum class SlotStatus : std::uint8_t { unavailable, available, occupied };
enum class PinLocation : std::uint8_t { top_left, top_right, bottom_left, bottom_right, any };

struct Slot {
	std::vector<PinLocation> pin_locations{};
	SlotStatus status{};
	std::optional<std::string> item{};
	std::optional<std::size_t> index{};
};

class Layout {
  public:
	Layout(dj::Json const& in);
	void serialize(dj::Json& out) const;

	bool place_item(std::string_view tag, PinLocation pin, std::size_t where); // returns true if placement was successful
	void remove_item(std::size_t const index);

	[[nodiscard]] auto can_place_item(PinLocation pin, std::size_t where) const -> bool;
	[[nodiscard]] auto has_item(std::string_view tag) const -> bool;
	[[nodiscard]] auto is_empty() const -> bool;
	[[nodiscard]] auto get_slots() const -> std::array<Slot, layout_size_v> const& { return m_slots; }
	[[nodiscard]] auto get_number_of_slots() const -> int;
	[[nodiscard]] auto has_matching_pins(Layout const& other) const -> bool;

	std::vector<std::string> get_items() const;

  private:
	std::array<Slot, layout_size_v> m_slots{};
};

} // namespace fornani::arms
