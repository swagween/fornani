
#pragma once

#include <SFML/Graphics.hpp>
#include <djson/json.hpp>
#include <string_view>

namespace fornani::automa {

struct MenuTheme {
	MenuTheme() = default;
	MenuTheme(dj::Json const& in);
	std::string label{};
	sf::Color backdrop{};
	sf::Color primary_text_color{};
	sf::Color secondary_text_color{};
	sf::Color activated_text_color{};
	sf::Color deactivated_text_color{};
	sf::Color dot_color{};
	int title_index{};
};

struct ServiceProvider;
struct Option {
	Option(ServiceProvider& svc, MenuTheme& theme, std::string_view lbl);
	void set_string(std::string_view str);
	sf::Text label;
	sf::Vector2f position{};
	sf::Vector2f left_offset{};
	sf::Vector2f right_offset{};
	sf::Vector2f dot_offset{24.f, 2.f};
	int index{};
	bool highlighted{};
	bool selected{};
	bool flagged{};
	bool selectable;
	void update(ServiceProvider& svc, int selection);

  private:
	MenuTheme* m_theme;
};
} // namespace fornani::automa
