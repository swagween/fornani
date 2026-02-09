
#include "fornani/automa/Option.hpp"
#include "fornani/automa/GameState.hpp"
#include "fornani/graphics/Colors.hpp"
#include "fornani/service/ServiceProvider.hpp"

namespace fornani::automa {

Option::Option(ServiceProvider& svc, MenuTheme& theme, std::string_view lbl) : label(svc.text.fonts.title), selectable{true}, m_theme{&theme} {
	label.setString(std::string{lbl});
	label.setCharacterSize(16);
	label.setLetterSpacing(1.f);
	label.setFillColor(theme.primary_text_color);
	label.setOrigin(label.getLocalBounds().getCenter());
}

void Option::set_string(std::string_view str) { label.setString(str.data()); }

void Option::update(int selection) {
	label.setPosition(position);
	left_offset = position - sf::Vector2f{label.getLocalBounds().getCenter().x + dot_offset.x - 2, -dot_offset.y};
	right_offset = position + sf::Vector2f{label.getLocalBounds().getCenter().x + dot_offset.x, dot_offset.y};
	selection == index ? label.setFillColor(m_theme->primary_text_color) : label.setFillColor(m_theme->secondary_text_color);
	if (flagged) { label.setFillColor(m_theme->activated_text_color); }
	if (!selectable) { label.setFillColor(m_theme->deactivated_text_color); }
	label.setOrigin(label.getLocalBounds().getCenter());
}

MenuTheme::MenuTheme(dj::Json const& in) {
	// metadata
	label = in["label"].as_string();
	title_index = in["index"].as<int>();

	// colors
	backdrop = sf::Color{in["backdrop"][0].as<std::uint8_t>(), in["backdrop"][1].as<std::uint8_t>(), in["backdrop"][2].as<std::uint8_t>()};
	primary_text_color = sf::Color{in["primary"][0].as<std::uint8_t>(), in["primary"][1].as<std::uint8_t>(), in["primary"][2].as<std::uint8_t>()};
	secondary_text_color = sf::Color{in["secondary"][0].as<std::uint8_t>(), in["secondary"][1].as<std::uint8_t>(), in["secondary"][2].as<std::uint8_t>()};
	activated_text_color = sf::Color{in["activated"][0].as<std::uint8_t>(), in["activated"][1].as<std::uint8_t>(), in["activated"][2].as<std::uint8_t>()};
	deactivated_text_color = sf::Color{in["deactivated"][0].as<std::uint8_t>(), in["deactivated"][1].as<std::uint8_t>(), in["deactivated"][2].as<std::uint8_t>()};
	dot_color = sf::Color{in["dot"][0].as<std::uint8_t>(), in["dot"][1].as<std::uint8_t>(), in["dot"][2].as<std::uint8_t>()};
}

} // namespace fornani::automa
