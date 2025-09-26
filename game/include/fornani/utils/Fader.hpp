#pragma once

#include <SFML/Graphics.hpp>
#include <fornani/utils/Cooldown.hpp>
#include <fornani/utils/Counter.hpp>
#include <array>
#include <unordered_map>

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani::util {

class Fader {
  public:
	Fader(automa::ServiceProvider& svc, int time, std::string_view color);
	void update();
	sf::Sprite& get_sprite() { return sprite; }

  private:
	int order{};
	int lookup{};
	int time{};
	int interval{};
	Counter progress{};
	sf::Sprite sprite;
	Cooldown timer{};
	std::unordered_map<std::string_view, std::array<int, 2>> color_codes{{"periwinkle", {0, 4}}, {"green", {1, 4}}, {"fucshia", {2, 4}}, {"yellow", {3, 4}}, {"night", {4, 4}},	 {"standard", {5, 4}}, {"save", {6, 5}},
																		 {"mythic", {7, 4}},	 {"fire", {8, 5}},	{"blue", {9, 4}},	 {"smoke", {10, 3}}, {"topaz", {11, 8}}, {"dark", {12, 4}},	   {"guardian", {13, 4}}};
};

} // namespace fornani::util
