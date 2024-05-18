
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

enum class Abilities { dash, wall_slide, double_jump, respiration, shield };

class AbilityManager {
  public:
	void update(automa::ServiceProvider& svc);
	void give_ability(Abilities ability);
	void give_ability(std::string_view ability);
	void remove_ability(Abilities ability);
	void clear();
	[[nodiscard]] auto has_ability(Abilities ability) const -> bool { return ability_flags.test(ability); }

  private:
	util::BitFlags<Abilities> ability_flags{};
	std::unordered_map<std::string_view, Abilities> ability_from_label{{"dash", Abilities::dash}, {"wall slide", Abilities::dash}, {"shield", Abilities::shield}, {"double jump", Abilities::double_jump}, {"respiration", Abilities::respiration}};
};

} // namespace player
