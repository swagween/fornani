
#pragma once

#include <string_view>

namespace fornani {

struct TransparentHash {
	using is_transparent = void;

	size_t operator()(std::string_view sv) const noexcept { return std::hash<std::string_view>{}(sv); }
};

struct TransparentEqual {
	using is_transparent = void;

	bool operator()(std::string_view a, std::string_view b) const noexcept { return a == b; }
};

} // namespace fornani
