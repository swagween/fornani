
#pragma once

namespace fornani::debug {

enum class PresentationMode { production, greyblock, debug, END };

constexpr inline auto presentation_mode_count = static_cast<int>(debug::PresentationMode::END);

inline auto cycle_presentation_mode(debug::PresentationMode mode, int direction) {
	auto value = static_cast<int>(mode) + direction;
	if (value < 0) { value = presentation_mode_count - 1; }
	if (value >= presentation_mode_count) { value = 0; }
	return static_cast<debug::PresentationMode>(value);
}

inline PresentationMode mode{};

inline int draw_calls{};

} // namespace fornani::debug
