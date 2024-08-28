#include "TextManager.hpp"

namespace data {

void TextManager::load_data() {
	//test
	console = dj::Json::from_file((finder.resource_path + "/text/console/basic.json").c_str());
	assert(!console.is_null());

	//inspectables
	inspectables = dj::Json::from_file((finder.resource_path + "/text/console/inspectables.json").c_str());
	assert(!inspectables.is_null());

	// basic
	basic = dj::Json::from_file((finder.resource_path + "/text/console/basic.json").c_str());
	assert(!basic.is_null());

	// NPCs
	npc = dj::Json::from_file((finder.resource_path + "/text/console/npc.json").c_str());
	assert(!npc.is_null());

	//set font
	text_font = finder.resource_path + "/text/fonts/pixelFJ8pt1.ttf";
	title_font = finder.resource_path + "/text/fonts/kongtext.ttf";
}

} // namespace data
