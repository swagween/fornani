
#pragma once

#include <djson/json.hpp>
#include <string>

namespace fornani::data {

struct MapData {
	int id{};
	dj::Json metadata{};
	std::string region_label{};
	std::string biome_label{};
	std::string room_label{};
};

struct MapTemplate {
	dj::Json metadata{};
	std::string region_label{};
	std::string biome_label{};
};

} // namespace fornani::data
