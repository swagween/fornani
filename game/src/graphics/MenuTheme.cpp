
#include <fornani/graphics/MenuTheme.hpp>

namespace fornani {

MenuTheme::MenuTheme(dj::Json const& in) {
	// metadata
	label = in["label"].as_string();
	title_index = in["index"].as<int>();

	// colors
	backdrop = Color{in["backdrop"]};
	primary_text_color = Color{in["primary"]};
	secondary_text_color = Color{in["secondary"]};
	activated_text_color = Color{in["activated"]};
	deactivated_text_color = Color{in["deactivated"]};
	dot_color = Color{in["dot"]};
}

} // namespace fornani
