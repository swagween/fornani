
#pragma once

#include "TextManager.hpp"
#include "ServiceLocator.hpp"

namespace data {

void TextManager::load_data() {

	//test
	console = dj::Json::from_file((finder.resource_path + "/text/console/basic.json").c_str());
	std::cout << "loading test console data ...";
	assert(!console.is_null());
	std::cout << " success!\n";

	//set font
	font = finder.resource_path + "/text/fonts/Connection.otf";

}

} // namespace data
