
#pragma once

#include <djson/json.hpp>
#include <array>
#include <optional>

namespace fornani::arms {

constexpr std::size_t layout_size_v = 5;

enum class SlotStatus : std::uint8_t { unavailable, deactivated, activated, occupied };
enum class PinLocation : std::uint8_t { top_left, top_right, bottom_left, bottom_right };

struct Slot {
	PinLocation pin_location{};
	SlotStatus status{};
	std::optional<std::string> item{};
};

class Layout {
  public:
	Layout(dj::Json const& in);
	void serialize(dj::Json& out) const;

	std::vector<std::string> get_items() const;

  private:
	std::array<Slot, layout_size_v> m_slots{};
};

} // namespace fornani::arms
