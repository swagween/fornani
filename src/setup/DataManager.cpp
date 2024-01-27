//
//  DataManager.hpp
//
//  Created by Alex Frasca on 12/26/22.
//

#pragma once

#include "DataManager.hpp"

namespace data {

	void DataManager::load_data() {

		frdog = dj::Json::from_file((finder.resource_path + "/data/enemy/frdog.json").c_str());
		assert(!frdog.is_null());
		hulmet = dj::Json::from_file((finder.resource_path + "/data/enemy/hulmet.json").c_str());
		assert(!frdog.is_null());
		
	
	}

}

/* DataManager_hpp */
