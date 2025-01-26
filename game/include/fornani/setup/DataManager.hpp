

#pragma once

#include <assert.h>
#include <SFML/Graphics.hpp>
#include <djson/json.hpp>
#include <array>
#include <iostream>
#include <string>
#include "fornani/level/Map.hpp"
#include "fornani/utils/QuestCode.hpp"
#include "File.hpp"
#include "ResourceFinder.hpp"

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
};

struct EnemyState {
	std::pair<int, int> code{};
	int respawn_distance{};
	bool permanent{};
};

class DataManager {

  public:
	DataManager(automa::ServiceProvider& svc, char** argv);
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
	void set_npc_location(int npc_id, int room_id);
	void kill_enemy(int room_id, int id, int distance, bool permanent);
	void respawn_enemy(int room_id, int id);
	void respawn_enemies(int room_id, int distance);
	void respawn_all();
	bool door_is_unlocked(int id) const;
	bool chest_is_open(int id) const;
	bool switch_is_activated(int id) const;
	bool block_is_destroyed(int id) const;
	bool inspectable_is_destroyed(std::string_view id) const;
	bool room_discovered(int id) const;
	bool enemy_is_fallen(int room_id, int id) const;

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
	int get_npc_location(int npc_id);
	std::vector<world::Layer>& get_layers(int id);

	dj::Json weapon{};
	dj::Json enemy_weapon{};
	dj::Json drop{};
	dj::Json particle{};
	dj::Json sparkler{};
	dj::Json map_styles{};
	dj::Json npc{};
	dj::Json item{};
	dj::Json platform{};
	dj::Json cutscene{};
	dj::Json action_names{};

	// enemy
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
	std::vector<int> rooms{};
	std::vector<int> discovered_rooms{};

	automa::ServiceProvider* m_services;
	std::unordered_map<int, npc::Vendor> marketplace{};
	std::unordered_map<int, int> npc_locations{};
	std::vector<EnemyState> fallen_enemies{};

  private:
	std::vector<int> opened_chests{};
	std::vector<int> unlocked_doors{};
	std::vector<int> activated_switches{};
	std::vector<int> destroyed_blocks{};
	std::vector<std::string> destroyed_inspectables{};
	std::vector<util::QuestKey> quest_progressions{};
};

} // namespace data
