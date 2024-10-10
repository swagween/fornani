
#pragma once

#include <vector>
#include "../player/Inventory.hpp"

namespace automa {
struct ServiceProvider;
}

namespace npc {
enum class VendorType { surveyor, gunsmith, engineer, fortuneteller };
/* NPCs will optionally have this package */
class Vendor {
  public:
	void set_upcharge(float to_upcharge) { upcharge = to_upcharge; }
	[[nodiscard]] auto get_upcharge() const -> float { return upcharge; }
	player::Inventory inventory{};
  private:
	std::vector<int> possible_items{};
	float upcharge{0.2f};
};
} // namespace npc