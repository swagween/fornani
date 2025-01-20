#pragma once
#include <cassert>
#include <string_view>
#include <array>
#include <unordered_map>
#include <../../../out/vs22-internal/_deps/sfml-src/include/SFML/Graphics.hpp>
#include "Cooldown.hpp"
#include "Counter.hpp"

namespace automa {
struct ServiceProvider;
}

namespace util {

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
	util::Counter progress{};
	sf::Sprite sprite;
	util::Cooldown timer{};
	std::unordered_map<std::string_view, std::array<int, 2>> color_codes{{"periwinkle", {0, 4}}, {"green", {1, 4}}, {"fucshia", {2, 4}}, {"yellow", {3, 4}}, {"night", {4, 4}}, {"standard", {5, 4}}, {"save", {6, 5}},
																		 {"mythic", {7, 4}},	 {"fire", {8, 5}},	{"blue", {9, 4}},	 {"smoke", {10, 3}}, {"topaz", {11, 8}}, {"dark", {12, 4}}};
};

} // namespace util