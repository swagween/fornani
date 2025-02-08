
#pragma once

#include <SFML/Graphics/Text.hpp>
#include <djson/json.hpp>
#include "ResourceFinder.hpp"

namespace fornani::data {

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
