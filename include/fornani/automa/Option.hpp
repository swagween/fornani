#pragma once
#include <SFML/Graphics.hpp>
#include <string_view>

namespace automa {
struct ServiceProvider;
struct Option {
	Option(ServiceProvider& svc, std::string_view lbl, bool red = false);
	void set_string(std::string_view str);
	sf::Text label;
	sf::Vector2<float> position{};
	sf::Vector2<float> left_offset{};
	sf::Vector2<float> right_offset{};
	sf::Vector2<float> dot_offset{24.f, 2.f};
	int index{};
	bool highlighted{};
	bool selected{};
	bool flagged{};
	bool red{};
	void update(ServiceProvider& svc, int selection);
};
} // namespace automa
