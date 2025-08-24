
#pragma once

#include <SFML/Graphics/Text.hpp>
#include <djson/json.hpp>
#include <fornani/setup/ResourceFinder.hpp>

namespace fornani::data {

class TextManager {
  public:
	explicit TextManager(ResourceFinder& finder);

	// jsons
	dj::Json console{};
	dj::Json inspectables{};
	dj::Json basic{};
	dj::Json item{};
	dj::Json npc{};

	struct {
		std::filesystem::path title{};
		std::filesystem::path basic{};
		std::filesystem::path config{};
	} sources{};
	struct {
		sf::Font title{};
		sf::Font basic{};
		sf::Font config{};
	} fonts{};
};

} // namespace fornani::data
