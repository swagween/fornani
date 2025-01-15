
#pragma once

#include <SFML/Graphics.hpp>
#include "../utils/Counter.hpp"
#include "../utils/Cooldown.hpp"
#include "../utils/BitFlags.hpp"
#include <string_view>
#include <unordered_map>
#include <vector>

namespace automa {
struct ServiceProvider;
}
namespace gui {
class Console;
}
namespace world {
class Map;
}
namespace player {
class Player;
}
namespace fornani {

enum class CutsceneFlags { complete };

class Cutscene {
  public:
	Cutscene(automa::ServiceProvider& svc, int id, std::string_view label);

	virtual void update(automa::ServiceProvider& svc, gui::Console& console, world::Map& map, player::Player& player) {};
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