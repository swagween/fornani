#pragma once

#include <SFML/Graphics.hpp>
#include "../utils/Counter.hpp"
#include <string_view>
#include <unordered_map>
#include <vector>

namespace fornani {

struct StatTracker {
	
	struct {
		util::Counter death_count{};
		util::Counter items_collected{};
		util::Counter guns_collected{};
		util::Counter bullets_fired{};
	} player{};

	struct {
		util::Counter total_orbs_collected{};
		util::Counter blue_orbs{};
		util::Counter highest_indicator_amount{};
	} treasure{};

	struct {
		util::Counter enemies_killed{};
	} enemy{};

	struct {
		util::Counter rooms_discovered{};
	} world{};

};

} // namespace fornani