
#pragma once

#include <djson/json.hpp>
#include <fornani/graphics/Color.hpp>
#include <string>

namespace fornani {

struct MenuTheme {
	MenuTheme() = default;
	MenuTheme(dj::Json const& in);
	std::string label{};
	Color backdrop{};
	Color primary_text_color{};
	Color secondary_text_color{};
	Color activated_text_color{};
	Color deactivated_text_color{};
	Color dot_color{};
	int title_index{};
};

} // namespace fornani
