
#pragma once
#include <cstdint>

namespace fornani {

namespace gui {

enum class MessageCodeType {
	none,
	response,
	item,
	quest,
	voice,
	emotion,
	redirect,
	action,
	exit,
	destructible,
	input_hint,
	reveal_item,
	start_battle,
	pop_conversation,
	play_song,
	weapon,
	remove_weapon,
	remove_item,
	destroy_inspectable,
	piggyback,
	open_vendor,
	launch_cutscene
};

enum class CodeSource { suite, response };

constexpr auto null_key = "null";

} // namespace gui

namespace world {

constexpr int CEIL_SLANT_INDEX = 448;
constexpr int FLOOR_SLANT_INDEX = 464;
constexpr int ceiling_single_ramp = 480;
constexpr int floor_single_ramp = 496;

} // namespace world

namespace arms {
enum class Team { nani, skycorps, guardian, pioneer, beast };
}

namespace item {
enum class Rarity { common, uncommon, rare, priceless };
}

constexpr auto gem_chance_v = 0.08f;
constexpr auto heart_chance_v = 8.f;
constexpr auto enemy_limit_v = 64;

enum class TimeOfDay { day, twilight, night, END };
enum class ClockMode { standard, military };

[[nodiscard]] constexpr static auto num_cycles() -> int { return static_cast<int>(TimeOfDay::END); }

} // namespace fornani
