
#pragma once

#include <assert.h>
#include <SFML/Graphics/Text.hpp>
#include <djson/json.hpp>
#include <iostream>
#include <string>
#include "ResourceFinder.hpp"

namespace data {

class TextManager {

	public:
	void load_data();

	//jsons
	dj::Json console{};

	dj::Json npc_test{};
	dj::Json npc_bryn{};


	std::string font{};

	ResourceFinder finder{};
};

} // namespace data
