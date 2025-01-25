
#pragma once

#include <string>
#include <unordered_map>

namespace pi {

inline std::unordered_map<int, char const*> layer_name{
	{0, "Background 0"}, {1, "Background 1"}, {2, "Background 2"}, {3, "Background 3"}, {4, "Middleground / Collidable"}, {5, "Foreground 1"}, {6, "Foreground 2"}, {7, "Foreground 3 / Obscuring"},
};

}

