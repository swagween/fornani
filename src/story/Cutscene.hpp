
#pragma once

#include <SFML/Graphics.hpp>
#include "../utils/Counter.hpp"
#include <string_view>
#include <unordered_map>
#include <vector>

namespace automa {
struct ServiceProvider;
}
namespace fornani {

	struct StoryAction{
	int npc_id{};
		int animation{};
	int walking_direction{};
	};

	class Cutscene {
  private:
		std::vector<int> npc_ids{};
	std::vector<int> map_ids{};
		std::vector<StoryAction> action_list{};
	};

} // namespace fornani