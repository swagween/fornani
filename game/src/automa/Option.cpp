
#include "fornani/automa/Option.hpp"
#include <fornani/graphics/MenuTheme.hpp>
#include "fornani/automa/GameState.hpp"
#include "fornani/graphics/Colors.hpp"
#include "fornani/service/ServiceProvider.hpp"

namespace fornani::automa {

Option::Option(ServiceProvider& svc, MenuTheme& theme, std::string_view lbl) : label(svc.text.fonts.title.font), selectable{true}, m_theme{&theme} {
	label.setString(std::string{lbl});
	label.setCharacterSize(16);
	label.setLetterSpacing(1.f);
	label.setFillColor(theme.primary_text_color);
	label.setOrigin(label.getLocalBounds().getCenter());
}

void Option::set_string(std::string_view str) { label.setString(str.data()); }

void Option::update(int selection, TextJustification justification) {
	label.setPosition(position);
	selection == index ? label.setFillColor(m_theme->primary_text_color) : label.setFillColor(m_theme->secondary_text_color);
	if (flagged) { label.setFillColor(m_theme->activated_text_color); }
	if (!selectable) { label.setFillColor(m_theme->deactivated_text_color); }
	auto origin = label.getLocalBounds().getCenter();
	if (justification == TextJustification::left) { origin.x = 0.f; }
	if (justification == TextJustification::right) { origin.x = label.getLocalBounds().size.x; }
	label.setOrigin(origin);
}

} // namespace fornani::automa
