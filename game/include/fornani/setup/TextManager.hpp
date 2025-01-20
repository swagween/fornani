
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
	TextManager(ResourceFinder& finder);

	//jsons
	dj::Json console{};
	dj::Json inspectables{};
	dj::Json basic{};

	dj::Json npc{};

	struct {
		std::filesystem::path title{};
		std::filesystem::path basic{};
	} sources{};
	struct {
		sf::Font title{};
		sf::Font basic{};
	} fonts{};
};

} // namespace data
