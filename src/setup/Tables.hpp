
#pragma once

#include <SFML/Graphics.hpp>
#include <string_view>
#include <unordered_map>

namespace lookup {

class Tables {
  public:
	std::unordered_map<int, std::string_view> gun_label{{0, "bryn's gun"}, {1, "plasmer"}, {2, "skycorps ar"}, {3, "tomahawk"}, {4, "grappling hook"}, {5, "grenade launcher"}, {6, "minigun"}};
	std::unordered_map<int, std::string_view> npc_label{{0, "bryn"}, {1, "haunch"}, {2, "willet"}, {3, "gobe"}, {4, "betty"}, {5, "nani"}, {6, "spencer"}, {7, "minigus"}};
};

} // namespace lookup
