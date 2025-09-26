
#pragma once
#include <cstdint>

namespace fornani {

namespace gui {

enum class MessageCodeType : std::uint8_t { none, response, item, quest, voice, emotion, redirect, action, exit, destructible, input_hint, reveal_item, start_battle, pop_conversation, play_song };
enum class CodeSource : std::uint8_t { suite, response };

constexpr auto null_key = "null";

} // namespace gui

constexpr auto gem_chance_v = 0.08f;
constexpr auto heart_chance_v = 8.f;
constexpr auto enemy_limit_v = 64;

} // namespace fornani
