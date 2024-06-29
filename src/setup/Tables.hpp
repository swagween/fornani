
#pragma once

#include <SFML/Graphics.hpp>
#include <string_view>
#include <unordered_map>

namespace lookup {

class Tables {
  public:
	std::unordered_map<int, std::string> get_map_label{};
	std::unordered_map<int, std::string_view> gun_label{{0, "bryn's gun"}, {1, "plasmer"},	{2, "skycorps ar"}, {3, "tomahawk"}, {4, "grappling hook"}, {5, "grenade launcher"},
														{6, "minigun"},	   {7, "soda gun"}, {8, "staple gun"},	{9, "indie"},	 {10, "gnat"}};
	std::unordered_map<int, std::string_view> npc_label{{0, "bryn"},   {1, "haunch"},  {2, "willet"}, {3, "gobe"},	{4, "betty"},  {5, "nani"},	  {6, "spencer"}, {7, "minigus"}, {8, "brody"},	  {9, "danny"}, {10, "gaia"},
														{11, "disco"}, {12, "jeanne"}, {13, "rune"},  {14, "ruan"}, {15, "guppy"}, {16, "dr_go"}, {17, "carl"},	  {18, "mirin"},  {19, "yogurt"}, {20, "bit"}};
};

} // namespace lookup
