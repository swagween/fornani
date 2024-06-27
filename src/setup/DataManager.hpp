

#pragma once

#include <assert.h>
#include <SFML/Graphics.hpp>
#include <djson/json.hpp>
#include <iostream>
#include <string>
#include <array>
#include "ResourceFinder.hpp"
#include "File.hpp"

namespace automa {
struct ServiceProvider;
}

namespace config {
class ControllerMap;
}

namespace player {
class Player;
}

namespace data {

class DataManager {

	public:
	DataManager(automa::ServiceProvider& svc);
	// game save
	void load_data();
	void save_progress(player::Player& player, int save_point_id);
	std::string_view load_progress(player::Player& player, int const file, bool state_switch = false);
	std::string_view load_blank_save(player::Player& player, bool state_switch = false);
	dj::Json& get_save() { return files.at(current_save).save_data; }
	fornani::File& get_file() { return files.at(current_save); }

	// tweaking
	void load_player_params(player::Player& player);
	void save_player_params(player::Player& player);

	// support user-defined control mapping
	void load_controls(config::ControllerMap& controller);
	void save_controls(config::ControllerMap& controller);
	void reset_controls();

	dj::Json game_info{};

	dj::Json weapon{};
	dj::Json drop{};
	dj::Json particle{};
	dj::Json sparkler{};
	dj::Json map_styles{};
	dj::Json npc{};
	dj::Json item{};
	dj::Json platform{};

	//enemy
	dj::Json enemy{};
	dj::Json frdog{};
	dj::Json hulmet{};

	int current_save{};
	std::array<fornani::File, 3> files{};
	fornani::File blank_file{};

	dj::Json player_params{};
	dj::Json menu{};
	dj::Json controls{};
	dj::Json map_table{};
	dj::Json background{};

	ResourceFinder finder{};

	automa::ServiceProvider* m_services;
};

} // namespace data
