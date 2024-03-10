
#pragma once

#include <assert.h>
#include <SFML/Graphics.hpp>
#include <djson/json.hpp>
#include <iostream>
#include <string>
#include "ResourceFinder.hpp"

namespace data {

class DataManager {

	public:
	// game save
	void load_data();
	  void save_progress(services::ServiceLocator& svc, player::Player& player, int save_point_id);
	void load_progress(services::ServiceLocator& svc, player::Player& player, int const file, bool state_switch = false);
	  void load_blank_save(services::ServiceLocator& svc, player::Player& player, bool state_switch = false);

	// tweaking
	  void load_player_params(player::Player& player);
	  void save_player_params(player::Player& player);

	dj::Json weapon{};

	//enemy
	dj::Json frdog{};
	dj::Json hulmet{};

	dj::Json save{};
	int current_save{};

	dj::Json player_params{};
	dj::Json map_table{};

	ResourceFinder finder{};
};

} // namespace data
