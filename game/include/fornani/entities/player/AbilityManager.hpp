
#pragma once

#include "fornani/utils/BitFlags.hpp"

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani::gui {
class Console;
}

namespace fornani::player {

enum class AbilityType { jump, slide, roll, wallslide, vision, dash, doublejump, dash_kick, double_dash, omnidirectional_dash, walljump, invalid };

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
