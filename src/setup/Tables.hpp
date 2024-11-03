
#pragma once

#include <SFML/Graphics.hpp>
#include <string_view>
#include <unordered_map>
#include <algorithm>
#include <cctype>

namespace lookup {

class Tables {
  public:
	std::unordered_map<int, std::string> get_map_label{};
	std::unordered_map<int, std::string_view> item_labels{};
	std::unordered_map<int, std::string_view> npc_label{{0, "bryn"},   {1, "haunch"}, {2, "willet"}, {3, "gobe"},	 {4, "betty"},	 {5, "nani"},	{6, "spencer"},		 {7, "minigus"},
														{8, "brody"},  {9, "danny"},  {10, "gaia"},	 {11, "disco"},	 {12, "jeanne"}, {13, "rune"},	{14, "ruan"},		 {15, "guppy"}, {16, "dr_go"},  {17, "carl"},		{18, "mirin"},	 {19, "yogurt"},
														{20, "bit"},   {21, "farah"},  {22, "lady_nimbus"}, {23, "hologus"}, {24, "justin"}};

	std::string get_npc_label_formatted(int id) {
		std::string ret = npc_label.at(id).data();
		std::transform(ret.begin(), ret.end(), ret.begin(), ::toupper);
		return ret;
	}
};

} // namespace lookup
