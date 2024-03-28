
#pragma once

#include <assert.h>
#include <SFML/Graphics.hpp>
#include <djson/json.hpp>
#include <iostream>
#include <string>
#include "ResourceFinder.hpp"

namespace player {
class Player;
}

namespace data {

class DataManager {

	public:
	// game save
	void load_data();
	void save_progress(player::Player& player, int save_point_id);
	std::string_view load_progress(player::Player& player, int const file, bool state_switch = false);
	std::string_view load_blank_save(player::Player& player, bool state_switch = false);

	// tweaking
	void load_player_params(player::Player& player);
	void save_player_params(player::Player& player);

	dj::Json weapon{};
	dj::Json drop{};
	dj::Json particle{};
	dj::Json sparkler{};
	dj::Json map_styles{};
	dj::Json npc{};

	//enemy
	dj::Json enemy{};
	dj::Json frdog{};
	dj::Json hulmet{};

	dj::Json save{};
	int current_save{};

	dj::Json player_params{};
	dj::Json map_table{};
	dj::Json background{};

	ResourceFinder finder{};
};

} // namespace data
