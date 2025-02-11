
#pragma once

#include <vector>
#include "fornani/entities/player/Inventory.hpp"

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani::npc {
enum class VendorType : uint8_t { surveyor, gunsmith, engineer, fortuneteller };
/* NPCs will optionally have this package */
class Vendor {
  public:
	void generate_inventory(automa::ServiceProvider& svc);
	void set_upcharge(float to_upcharge) { upcharge = to_upcharge; }
	[[nodiscard]] auto get_upcharge() const -> float { return upcharge; }
	player::Inventory inventory{};
	std::vector<int> common_items{};
	std::vector<int> uncommon_items{};
	std::vector<int> rare_items{};
  private:
	float upcharge{0.2f};
	int stock_size{6};
};
} // namespace fornani::npc
