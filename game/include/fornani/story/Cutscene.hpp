
#pragma once

#include <SFML/Graphics.hpp>
#include "fornani/utils/Counter.hpp"
#include "fornani/utils/Cooldown.hpp"
#include "fornani/utils/BitFlags.hpp"
#include <string_view>
#include <unordered_map>
#include <vector>

namespace fornani::automa {
struct ServiceProvider;
}
namespace fornani::gui {
class Console;
}
namespace fornani::world {
class Map;
}
namespace fornani::player {
class Player;
}
namespace fornani {

enum class CutsceneFlags { complete };

class Cutscene {
  public:
	Cutscene(automa::ServiceProvider& svc, int id, std::string_view label);

	virtual void update([[maybe_unused]] automa::ServiceProvider& svc, [[maybe_unused]] gui::Console& console, [[maybe_unused]] world::Map& map, [[maybe_unused]] player::Player& player){};
	[[nodiscard]] auto complete() const -> bool { return flags.test(CutsceneFlags::complete); }

  protected:
	util::BitFlags<CutsceneFlags> flags{};
	int progress{};
	int total_conversations{};
	int id{};
	struct {
		bool no_player{};
		int target_state_on_end{};
	} metadata{};
	struct {
		util::Cooldown beginning{256};
		util::Cooldown pause{64};
		util::Cooldown long_pause{512};
		util::Cooldown progressor{16};
		util::Cooldown end{256};
	} cooldowns{};

	// debug
	bool debug{};
};

} // namespace fornani