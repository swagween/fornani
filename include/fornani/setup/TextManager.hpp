
#pragma once

#include <assert.h>
#include <../../../out/vs22-internal/_deps/sfml-src/include/SFML/Graphics/Text.hpp>
#include <../../../out/vs22-internal/_deps/djson-src/include/djson/json.hpp>
#include <iostream>
#include <string>
#include "ResourceFinder.hpp"

namespace data {

class TextManager {

	public:
	TextManager(ResourceFinder& finder);

	//jsons
	dj::Json console{};
	dj::Json inspectables{};
	dj::Json basic{};

	dj::Json npc{};

	std::string text_font{};
	std::string title_font{};

	struct {
		sf::Font title{};
		sf::Font basic{};
	} fonts{};
};

} // namespace data
