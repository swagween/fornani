
#pragma once

#include <SFML/Graphics.hpp>
#include <string_view>
#include <unordered_map>

namespace lookup {

	class Tables {
  public:
		std::unordered_map<int, std::string_view> gun_label{{0, "bryn's gun"}, {1, "plasmer"}, {2, "skycorps ar"}, {3, "tomahawk"}, {4, "grappling hook"}};
	};

}
