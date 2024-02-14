//
//  DataManager.hpp
//
//  Created by Alex Frasca on 12/26/22.
//

#pragma once

#include "DataManager.hpp"
#include "LookupTables.hpp"
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

		save.dj::Json::to_file((finder.resource_path + "/data/save/file_0.json").c_str());

	}

	void DataManager::load_progress() {

		save = dj::Json::from_file((finder.resource_path + "/data/save/file_0.json").c_str());
		assert(!save.is_null());

	}

}

/* DataManager_hpp */
