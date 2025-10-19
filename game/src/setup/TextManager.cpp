#include "fornani/setup/TextManager.hpp"

namespace fornani::data {

TextManager::TextManager(ResourceFinder& finder)
	: sources{.title{finder.resource_path() + "/text/fonts/kongtext.ttf"}, .basic{finder.resource_path() + "/text/fonts/pixelFJ8pt1.ttf"}, .config{finder.resource_path() + "/text/fonts/Vera.ttf"}},
	  fonts{.title = sf::Font{sources.title}, .basic = sf::Font{sources.basic}, .config = sf::Font{sources.config}} {
	// test
	console = *dj::Json::from_file((finder.resource_path() + "/text/console/basic.json").c_str());
	assert(!console.is_null());

	// inspectables
	inspectables = *dj::Json::from_file((finder.resource_path() + "/text/console/inspectables.json").c_str());
	assert(!inspectables.is_null());

	// basic
	basic = *dj::Json::from_file((finder.resource_path() + "/text/console/basic.json").c_str());
	assert(!basic.is_null());

	// basic
	item = *dj::Json::from_file((finder.resource_path() + "/text/console/item.json").c_str());
	assert(!item.is_null());

	// NPCs
	npc = *dj::Json::from_file((finder.resource_path() + "/text/console/npc.json").c_str());
	assert(!npc.is_null());

	fonts.title.setSmooth(false);
	fonts.basic.setSmooth(false);
}

} // namespace fornani::data
