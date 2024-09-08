

#pragma once

#include <assert.h>
#include <SFML/Graphics.hpp>
#include <djson/json.hpp>
#include <iostream>
#include <string>
#include <array>
#include "ResourceFinder.hpp"
#include "File.hpp"
#include "../utils/QuestCode.hpp"
#include "../level/Map.hpp"

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

	struct MapData {
	int id{};
	dj::Json metadata{};
	dj::Json tiles{};
	dj::Json inspectable_data{};
	};

class DataManager {

	public:
	DataManager(automa::ServiceProvider& svc);
	// game save
	void load_data(std::string in_room = "");
	void save_progress(player::Player& player, int save_point_id);
	void save_settings();
	int load_progress(player::Player& player, int const file, bool state_switch = false, bool from_menu = true);
	void load_settings();
	void delete_file(int index);
	void write_death_count(player::Player& player);
	std::string_view load_blank_save(player::Player& player, bool state_switch = false);
	dj::Json& get_save() { return files.at(current_save).save_data; }
	fornani::File& get_file() { return files.at(current_save); }

	// tweaking
	void load_player_params(player::Player& player);
	void save_player_params(player::Player& player);

	// map-related save data helpers
	void open_chest(int id);
	void unlock_door(int id);
	void activate_switch(int id);
	void destroy_block(int id);
	void destroy_inspectable(std::string_view id);
	void push_quest(util::QuestKey key);
	bool door_is_unlocked(int id) const;
	bool chest_is_open(int id) const;
	bool switch_is_activated(int id) const;
	bool block_is_destroyed(int id) const;
	bool inspectable_is_destroyed(std::string_view id) const;
	bool room_discovered(int id) const;

	// support user-defined control mapping
	void load_controls(config::ControllerMap& controller);
	void save_controls(config::ControllerMap& controller);
	void reset_controls();

	[[nodiscard]] auto exists(int candidate) const -> bool {
		for (auto& room : rooms) {
			if (room == candidate) { return true; }
		}
		return false;
	}

	int get_room_index(int id);
	std::vector<world::Layer>& get_layers(int id);

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
	dj::Json settings{};
	dj::Json map_table{};
	dj::Json background{};

	std::vector<MapData> map_jsons{};
	std::vector<std::vector<world::Layer>> map_layers{};
	int num_layers{8};
	std::vector<int> rooms{0, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 199, 120, 121, 122, 123, 124, 125, 200, 224, 299, 3001, 3002, 6001};
	std::vector<int> discovered_rooms{};

	ResourceFinder finder{};

	automa::ServiceProvider* m_services;

  private:
	std::vector<int> opened_chests{};
	std::vector<int> unlocked_doors{};
	std::vector<int> activated_switches{};
	std::vector<int> destroyed_blocks{};
	std::vector<std::string> destroyed_inspectables{};
	std::vector<util::QuestKey> quest_progressions{};
};

} // namespace data
