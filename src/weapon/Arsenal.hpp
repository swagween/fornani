
#pragma once

#include <array>
#include <cstdio>
#include <list>
#include <memory>
#include <unordered_map>
#include "Weapon.hpp"
#include "../utils/Circuit.hpp"

namespace arms {

using Key = WEAPON_TYPE;
constexpr static int max_weapons{11};

struct Arsenal {

	Arsenal() = default;
	Arsenal(automa::ServiceProvider& svc);

	void push_to_loadout(int id);
	void pop_from_loadout(int id);
	void switch_weapon(automa::ServiceProvider& svc, int next);
	void set_index(int index);
	constexpr void clear() { loadout.clear(); }
	std::optional<std::reference_wrapper<std::unique_ptr<Weapon>>> get_weapon_at(int id);
	std::optional<std::reference_wrapper<std::unique_ptr<Weapon>>> get_current_weapon();
	constexpr std::vector<std::unique_ptr<Weapon>>& get_loadout() { return loadout; }
	[[nodiscard]] auto get_index() const -> size_t { return current_weapon.value().get(); }
	[[nodiscard]] auto size() const -> size_t { return loadout.size(); }
	[[nodiscard]] auto empty() const -> bool { return loadout.empty(); }
	bool has(int id);

  private:
	std::vector<std::unique_ptr<Weapon>> loadout{};
	std::optional<util::Circuit> current_weapon{};
	automa::ServiceProvider* m_services{};
};

} // namespace arms
