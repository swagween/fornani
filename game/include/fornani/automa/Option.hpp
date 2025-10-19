#pragma once

#include <SFML/Graphics.hpp>
#include <string_view>

namespace fornani::automa {
struct ServiceProvider;
struct Option {
	Option(ServiceProvider& svc, std::string_view lbl, bool red = false);
	void set_string(std::string_view str);
	sf::Text label;
	sf::Color native_color{};
	sf::Vector2f position{};
	sf::Vector2f left_offset{};
	sf::Vector2f right_offset{};
	sf::Vector2f dot_offset{24.f, 2.f};
	int index{};
	bool highlighted{};
	bool selected{};
	bool flagged{};
	bool red{};
	bool selectable;
	void update(ServiceProvider& svc, int selection);
};
} // namespace fornani::automa
