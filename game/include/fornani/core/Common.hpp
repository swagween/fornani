
#pragma once

#include <array>
#include <cstdint>
#include <string>

namespace fornani {

constexpr static std::uint32_t magic_key_v = 0x464F524E;
constexpr std::array<std::uint8_t, 16> save_key_v = {0xF3, 0x91, 0x2A, 0x7C, 0xD8, 0x44, 0xB1, 0x09, 0x6E, 0xAF, 0x33, 0xC2, 0x18, 0x5D, 0x77, 0xE0};
constexpr std::uint16_t save_version_v = 0;

constexpr int default_framerate_limit_v = 120;

namespace gui {

enum class OutputType { gradual, instant, no_exit, no_skip };
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
	launch_cutscene,
	add_map_marker,
	set_cutscene_progression,
	purchase,
	give_bonus_health,
	open_builder,
	set_quest_progression
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
constexpr auto max_damage_v = 32.f;

enum class DrawOrder { back, front };

enum class TimeOfDay { dawn, day, dusk, night, END };
enum class ClockMode { standard, military };

[[nodiscard]] constexpr static auto num_cycles() -> int { return static_cast<int>(TimeOfDay::END); }

} // namespace fornani
