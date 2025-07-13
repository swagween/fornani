
#pragma once
#include <fornani/entities/player/abilities/Dash.hpp>
#include <fornani/entities/player/abilities/Jump.hpp>
#include <fornani/entities/player/abilities/Roll.hpp>
#include <fornani/entities/player/abilities/Slide.hpp>
#include <fornani/entities/player/abilities/Wallslide.hpp>
#include "fornani/utils/BitFlags.hpp"

#include <memory>
#include <optional>

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani::gui {
class Console;
}

namespace fornani::player {

enum class AbilityType : std::uint8_t { wall_slide, dash, double_jump, respiration };

class AbilityManager {
  public:
	void give_ability(AbilityType ability);
	void give_ability(int ability);
	void remove_ability(AbilityType ability);
	void clear();
	[[nodiscard]] auto has_ability(AbilityType ability) const -> bool { return ability_flags.test(ability); }

  private:
	util::BitFlags<AbilityType> ability_flags{};
	std::optional<std::unique_ptr<Ability>> m_active_ability{};
};

} // namespace fornani::player
