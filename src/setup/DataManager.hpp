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

		dj::Json frdog{};
		dj::Json hulmet{};

		ResourceFinder finder{};
	
	};

}

/* LookupTables_hpp */
