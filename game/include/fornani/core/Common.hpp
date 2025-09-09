
#pragma once
#include <cstdint>

namespace fornani {

namespace gui {

enum class MessageCodeType : std::uint8_t { none, response, item, quest, voice, emotion, redirect, action, exit, destructible, input_hint, reveal_item, start_battle, pop_conversation, play_song };
enum class CodeSource : std::uint8_t { suite, response };

constexpr auto null_key = "null";

} // namespace gui

} // namespace fornani
