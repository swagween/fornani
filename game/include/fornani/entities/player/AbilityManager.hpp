
#pragma once
#include "fornani/utils/BitFlags.hpp"

#include <cstdint>

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani::gui {
class Console;
}

namespace fornani::player {

enum class Abilities : std::uint8_t { wall_slide, dash, double_jump, respiration, shield };

class AbilityManager {
  public:
	void give_ability(Abilities ability);
	void give_ability(int ability);
	void remove_ability(Abilities ability);
	void clear();
	[[nodiscard]] auto has_ability(Abilities ability) const -> bool { return ability_flags.test(ability); }

  private:
	util::BitFlags<Abilities> ability_flags{};
};

} // namespace fornani::player
