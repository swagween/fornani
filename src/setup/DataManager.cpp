//
//  DataManager.hpp
//
//  Created by Alex Frasca on 12/26/22.
//

#pragma once

#include "DataManager.hpp"
#include "LookupTables.hpp"
#include "MapLookups.hpp"
#include "ServiceLocator.hpp"

namespace data {

	void DataManager::load_data() {

		frdog = dj::Json::from_file((finder.resource_path + "/data/enemy/frdog.json").c_str());
		assert(!frdog.is_null());
		hulmet = dj::Json::from_file((finder.resource_path + "/data/enemy/hulmet.json").c_str());
		assert(!frdog.is_null());
		
	
	}

	void DataManager::save_progress(int save_point_id) {

		//set file data based on player state
		save["player_data"]["max_hp"] = svc::playerLocator.get().player_stats.max_health;
		save["player_data"]["hp"] = svc::playerLocator.get().player_stats.health;
		save["player_data"]["orbs"] = svc::playerLocator.get().player_stats.orbs;
		save["player_data"]["position"]["x"] = svc::playerLocator.get().collider.physics.position.x;
		save["player_data"]["position"]["y"] = svc::playerLocator.get().collider.physics.position.y;

		//save arsenal
		//wipe it first
		constexpr auto empty_array = R"([])";
		auto const wipe = dj::Json::parse(empty_array);
		save["player_data"]["arsenal"] = wipe;
		//push player arsenal
		for (auto& gun : svc::playerLocator.get().weapons_hotbar) {
			int this_id = lookup::type_to_index.at(gun);
			save["player_data"]["arsenal"].push_back(this_id);
		}
		save["player_data"]["equipped_gun"] = svc::playerLocator.get().loadout.get_equipped_weapon().get_id();

		save["save_point_id"] = save_point_id;

		save.dj::Json::to_file((finder.resource_path + "/data/save/file_" + std::to_string(current_save) +".json").c_str());

	}

	void DataManager::load_progress(const int file) {

		current_save = file;

		save = dj::Json::from_file((finder.resource_path + "/data/save/file_" + std::to_string(file) + ".json").c_str());
		assert(!save.is_null());

		int save_pt_id = svc::dataLocator.get().save["save_point_id"].as<int>();
		int room_id = lookup::save_point_to_room_id.at(save_pt_id);

		svc::stateControllerLocator.get().next_state = lookup::get_map_label.at(room_id);
		svc::stateControllerLocator.get().trigger = true;

		//set player data based on save file
		svc::playerLocator.get().player_stats.max_health = svc::dataLocator.get().save["player_data"]["max_hp"].as<int>();
		svc::playerLocator.get().player_stats.health = svc::dataLocator.get().save["player_data"]["hp"].as<int>();
		svc::playerLocator.get().player_stats.orbs = svc::dataLocator.get().save["player_data"]["orbs"].as<int>();

		//load player's arsenal
		svc::playerLocator.get().weapons_hotbar.clear();
		for (auto& gun_id : svc::dataLocator.get().save["player_data"]["arsenal"].array_view()) {
			svc::playerLocator.get().weapons_hotbar.push_back(lookup::index_to_type.at(gun_id.as<int>()));
		}
		if (!svc::playerLocator.get().weapons_hotbar.empty()) {
			auto equipped_gun = svc::dataLocator.get().save["player_data"]["equipped_gun"].as<int>();
			svc::playerLocator.get().loadout.equipped_weapon = lookup::index_to_type.at(equipped_gun);
		}

	}

	void DataManager::load_blank_save() {

		save = dj::Json::from_file((finder.resource_path + "/data/save/new_game.json").c_str());
		assert(!save.is_null());

		svc::stateControllerLocator.get().next_state = lookup::get_map_label.at(101);
		svc::stateControllerLocator.get().trigger = true;

		//set player data based on save file
		svc::playerLocator.get().player_stats.max_health = svc::dataLocator.get().save["player_data"]["max_hp"].as<int>();
		svc::playerLocator.get().player_stats.health = svc::dataLocator.get().save["player_data"]["hp"].as<int>();
		svc::playerLocator.get().player_stats.orbs = svc::dataLocator.get().save["player_data"]["orbs"].as<int>();

		//load player's arsenal
		svc::playerLocator.get().weapons_hotbar.clear();

	}

}

/* DataManager_hpp */
