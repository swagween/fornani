
#pragma once

#include <SFML/Graphics.hpp>
#include <djson/json.hpp>
#include <string_view>

namespace fornani {

struct FontSpec {
	sf::Font font{};
	int glyph_size{16};
	float line_spacing{1.5f};
	sf::Vector2f offset{};
};

static void set_utf8_string(dj::Json const& in, sf::Text& text) {
	std::string str = in.as_string();
	sf::String s = sf::String::fromUtf8(str.begin(), str.end());
	text.setString(s);
}

static void set_utf8_string(std::string_view in, sf::Text& text) {
	std::string str = in.data();
	sf::String s = sf::String::fromUtf8(str.begin(), str.end());
	text.setString(s);
}

static sf::Vector2f get_character_position(sf::Text& text, std::size_t index) {
	auto const& glyphs = text.getShapedGlyphs();
	if (glyphs.size() <= index) { return sf::Vector2f{}; }
	sf::Vector2f local_position = glyphs[index].position;
	return text.getTransform().transformPoint(local_position);
}

} // namespace fornani
