
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
	dj::Json inspectables{};
	dj::Json basic{};

	dj::Json npc{};

	std::string text_font{};
	std::string title_font{};

	struct {
		sf::Font title{};
		sf::Font basic{};
	} fonts{};

	ResourceFinder finder{};
};

} // namespace data
