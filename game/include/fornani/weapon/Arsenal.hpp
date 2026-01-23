
#pragma once

#include <memory>
#include "Weapon.hpp"

namespace fornani::arms {

class Arsenal {
  public:
	explicit Arsenal(automa::ServiceProvider& svc);

	void push_to_loadout(std::string_view tag);
	void pop_from_loadout(std::string_view tag);
	void reset();
	constexpr void clear() { loadout.clear(); }
	Weapon& get_weapon_at(std::string_view tag);
	std::vector<std::unique_ptr<Weapon>> const& get_loadout() const { return loadout; }
	[[nodiscard]] auto size() const -> std::size_t { return loadout.size(); }
	[[nodiscard]] auto empty() const -> bool { return loadout.empty(); }
	bool has(std::string_view tag);

  private:
	std::vector<std::unique_ptr<Weapon>> loadout{};
	automa::ServiceProvider* m_services{};
};

} // namespace fornani::arms
