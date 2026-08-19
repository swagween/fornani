
#pragma once

#include <vector>
#include "fornani/entities/player/Inventory.hpp"

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani::npc {
enum class VendorType : std::uint8_t { surveyor, gunsmith, engineer, fortuneteller, seamstress };
/* NPCs will optionally have this package */
class Vendor {
  public:
	Vendor(std::size_t stock, float upcharge) : m_stock_size{stock}, m_upcharge{upcharge} {}
	void generate_inventory(automa::ServiceProvider& svc);
	[[nodiscard]] auto get_upcharge() const -> float { return m_upcharge; }
	player::Inventory inventory{};
	std::vector<std::string> common_items{};
	std::vector<std::string> uncommon_items{};
	std::vector<std::string> rare_items{};
	std::vector<std::string> guaranteed_finite_items{};

  private:
	float m_upcharge{0.2f};
	std::size_t m_stock_size{6};
};
} // namespace fornani::npc
