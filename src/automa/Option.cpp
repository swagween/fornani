
#include "Option.hpp"
#include "GameState.hpp"
#include "../service/ServiceProvider.hpp"

namespace automa {

Option::Option(ServiceProvider& svc, std::string_view lbl, sf::Font& font) {
	label.setFont(font);
	label.setString(lbl.data());
	label.setCharacterSize(16);
	label.setLetterSpacing(title_letter_spacing);
	label.setFillColor(svc.styles.colors.ui_white);
	label.setOrigin(label.getLocalBounds().width * 0.5f, label.getLocalBounds().height * 0.5f);
}

void Option::update(ServiceProvider& svc, int selection) {
	label.setPosition(position);
	left_offset = position - sf::Vector2<float>{label.getLocalBounds().width * 0.5f + dot_offset.x - 2, -dot_offset.y};
	right_offset = position + sf::Vector2<float>{label.getLocalBounds().width * 0.5f + dot_offset.x, dot_offset.y};
	selection == index ? label.setFillColor(svc.styles.colors.ui_white) : label.setFillColor(svc.styles.colors.dark_grey);
	if (flagged) { label.setFillColor(svc.styles.colors.red); }
	label.setOrigin(label.getLocalBounds().width * 0.5f, label.getLocalBounds().height * 0.5f);
}

} // namespace automa
