
#pragma once

#include "fornani/utils/BitFlags.hpp"

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani::gui {
class Console;
}

namespace fornani::player {

enum class AbilityType : std::uint8_t { wallslide, dash, doublejump, respiration };

class AbilityManager {
  public:
	void give_ability(AbilityType ability);
	void give_ability(int ability);
	void remove_ability(AbilityType ability);
	void clear();
	[[nodiscard]] auto has_ability(AbilityType ability) const -> bool { return ability_flags.test(ability); }

  private:
	util::BitFlags<AbilityType> ability_flags{};
};

} // namespace fornani::player
