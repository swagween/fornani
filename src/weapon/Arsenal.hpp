
#pragma once

#include <array>
#include <cstdio>
#include <list>
#include <memory>
#include <unordered_map>
#include "Weapon.hpp"
#include "../utils/Circuit.hpp"

namespace arms {

class Arsenal {
  public:
	Arsenal(automa::ServiceProvider& svc);

	void push_to_loadout(int id);
	void pop_from_loadout(int id);
	constexpr void clear() { loadout.clear(); }
	Weapon& get_weapon_at(int id);
	constexpr std::vector<std::unique_ptr<Weapon>>& get_loadout() { return loadout; }
	[[nodiscard]] auto size() const -> size_t { return loadout.size(); }
	[[nodiscard]] auto empty() const -> bool { return loadout.empty(); }
	bool has(int id);

  private:
	std::vector<std::unique_ptr<Weapon>> loadout{};
	automa::ServiceProvider* m_services{};
};

} // namespace arms
