//
//  LookupTables.hpp
//
//  Created by Alex Frasca on 12/26/22.
//

#pragma once


#include <SFML/Graphics.hpp>
#include <djson/json.hpp>
#include <iostream>
#include <assert.h>
#include <string>
#include "ResourceFinder.hpp"

namespace data {

	class DataManager {

	public:

		void load_data();
		void save_progress(int save_point_id);
		void load_progress();

		dj::Json frdog{};
		dj::Json hulmet{};

		dj::Json save{};

		ResourceFinder finder{};
	
	};

}

/* LookupTables_hpp */
