
#pragma once
#include "../../utils/BitFlags.hpp"
#include <unordered_map>
#include <string_view>

namespace automa {
struct ServiceProvider;
}

namespace gui {
class Console;
}

namespace player {

enum class Abilities { wall_slide, dash, double_jump, respiration, shield };

class AbilityManager {
  public:
	void update(automa::ServiceProvider& svc);
	void give_ability(Abilities ability);
	void give_ability(int ability);
	void remove_ability(Abilities ability);
	void clear();
	[[nodiscard]] auto has_ability(Abilities ability) const -> bool { return ability_flags.test(ability); }

  private:
	util::BitFlags<Abilities> ability_flags{};
};

} // namespace player
