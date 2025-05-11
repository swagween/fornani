
#include "fornani/automa/Option.hpp"
#include "fornani/automa/GameState.hpp"
#include "fornani/graphics/Colors.hpp"
#include "fornani/service/ServiceProvider.hpp"

namespace fornani::automa {

Option::Option(ServiceProvider& svc, std::string_view lbl, bool red) : red(red), label(svc.text.fonts.title), selectable{true} {
	label.setString(lbl.data());
	label.setCharacterSize(16);
	label.setLetterSpacing(title_letter_spacing);
	red ? label.setFillColor(colors::red) : label.setFillColor(colors::ui_white);
	label.setOrigin(label.getLocalBounds().getCenter());
	native_color = colors::dark_grey;
}

void Option::set_string(std::string_view str) { label.setString(str.data()); }

void Option::update(ServiceProvider& svc, int selection) {
	label.setPosition(position);
	left_offset = position - sf::Vector2<float>{label.getLocalBounds().getCenter().x + dot_offset.x - 2, -dot_offset.y};
	right_offset = position + sf::Vector2<float>{label.getLocalBounds().getCenter().x + dot_offset.x, dot_offset.y};
	if (red) {
		selection == index ? label.setFillColor(colors::red) : label.setFillColor(colors::beige);
	} else {
		selection == index ? label.setFillColor(colors::ui_white) : label.setFillColor(native_color);
	}
	if (flagged) { label.setFillColor(colors::red); }
	if (!selectable) { label.setFillColor(colors::console_blue); }
	label.setOrigin(label.getLocalBounds().getCenter());
}

} // namespace fornani::automa
