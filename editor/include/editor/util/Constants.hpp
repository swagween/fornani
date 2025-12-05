
#pragma once

namespace pi {

enum class ToolType { brush, fill, marquee, erase, hand, entity_editor, eyedropper, END };

}

namespace pi::constants {

constexpr auto tool_size_v = 8;
constexpr auto ui_tool_scale_v = 3.f;

} // namespace pi::constants
