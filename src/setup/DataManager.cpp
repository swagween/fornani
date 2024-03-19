
#pragma once

#include "DataManager.hpp"
#include "LookupTables.hpp"
#include "MapLookups.hpp"
#include "ServiceLocator.hpp"

namespace data {

void DataManager::load_data() {

	//weapon
	weapon = dj::Json::from_file((finder.resource_path + "/data/weapon/weapon_data.json").c_str());
	std::cout << "loading weapon data ...";
	assert(!weapon.is_null());
	std::cout << " success!\n";

	drop = dj::Json::from_file((finder.resource_path + "/data/item/drop.json").c_str());
	std::cout << "loading drop data ...";
	assert(!drop.is_null());
	std::cout << " success!\n";

	particle = dj::Json::from_file((finder.resource_path + "/data/vfx/particle.json").c_str());
	std::cout << "loading particle data ...";
	assert(!particle.is_null());
	std::cout << " success!\n";

	// enemies
	enemy = dj::Json::from_file((finder.resource_path + "/data/enemy/enemy_params.json").c_str());
	std::cout << "loading enemy parameters ...";
	assert(!enemy.is_null());
	std::cout << " success!\n";

	frdog = dj::Json::from_file((finder.resource_path + "/data/enemy/frdog.json").c_str());
	std::cout << "loading frdog ...";
	assert(!frdog.is_null());
	std::cout << " success!\n";
	hulmet = dj::Json::from_file((finder.resource_path + "/data/enemy/hulmet.json").c_str());
	std::cout << "loading hulmet ...";
	assert(!frdog.is_null());
	std::cout << " success!\n";


	// map
	map_table = dj::Json::from_file((finder.resource_path + "/data/level/map_table.json").c_str());
	std::cout << "loading map data ...";
	assert(!map_table.is_null());
	for (auto const& room : map_table["rooms"].array_view()) { lookup::get_map_label.insert(std::make_pair(room["room_id"].as<int>(), room["label"].as_string())); }
	std::cout << " success!\n";
}

void DataManager::save_progress(int save_point_id) {

	// set file data based on player state
	save["player_data"]["max_hp"] = svc::playerLocator.get().player_stats.max_health;
	save["player_data"]["hp"] = svc::playerLocator.get().player_stats.health;
	save["player_data"]["orbs"] = svc::playerLocator.get().player_stats.orbs;
	save["player_data"]["position"]["x"] = svc::playerLocator.get().collider.physics.position.x;
	save["player_data"]["position"]["y"] = svc::playerLocator.get().collider.physics.position.y;

	// save arsenal
	// wipe it first
	constexpr auto empty_array = R"([])";
	auto const wipe = dj::Json::parse(empty_array);
	save["player_data"]["arsenal"] = wipe;
	// push player arsenal
	for (auto& gun : svc::playerLocator.get().arsenal.loadout) {
		int this_id = gun.get_id();
		save["player_data"]["arsenal"].push_back(this_id);
	}
	save["player_data"]["equipped_gun"] = svc::playerLocator.get().arsenal.get_index();

	save["save_point_id"] = save_point_id;

	save.dj::Json::to_file((finder.resource_path + "/data/save/file_" + std::to_string(current_save) + ".json").c_str());
}

std::string_view DataManager::load_progress(int const file, bool state_switch) {

	current_save = file;

	save = dj::Json::from_file((finder.resource_path + "/data/save/file_" + std::to_string(file) + ".json").c_str());
	assert(!save.is_null());

	int save_pt_id = svc::dataLocator.get().save["save_point_id"].as<int>();
	int room_id = lookup::save_point_to_room_id.at(save_pt_id);

	// set player data based on save file
	svc::playerLocator.get().player_stats.max_health = svc::dataLocator.get().save["player_data"]["max_hp"].as<int>();
	svc::playerLocator.get().player_stats.health = svc::dataLocator.get().save["player_data"]["hp"].as<int>();
	svc::playerLocator.get().player_stats.orbs = svc::dataLocator.get().save["player_data"]["orbs"].as<int>();

	// load player's arsenal
	svc::playerLocator.get().arsenal.loadout.clear();
	for (auto& gun_id : svc::dataLocator.get().save["player_data"]["arsenal"].array_view()) {
		svc::playerLocator.get().arsenal.push_to_loadout(gun_id.as<int>());
	}
	if (!svc::playerLocator.get().arsenal.loadout.empty()) {
		auto equipped_gun = svc::dataLocator.get().save["player_data"]["equipped_gun"].as<int>();
		svc::playerLocator.get().arsenal.set_index(equipped_gun);
	}

	//reset some things that might be lingering
	svc::playerLocator.get().arsenal.extant_projectile_instances = {};

	return lookup::get_map_label.at(room_id);
}

std::string_view DataManager::load_blank_save(bool state_switch) {

	save = dj::Json::from_file((finder.resource_path + "/data/save/new_game.json").c_str());
	assert(!save.is_null());

	// set player data based on save file
	svc::playerLocator.get().player_stats.max_health = svc::dataLocator.get().save["player_data"]["max_hp"].as<int>();
	svc::playerLocator.get().player_stats.health = svc::dataLocator.get().save["player_data"]["hp"].as<int>();
	svc::playerLocator.get().player_stats.orbs = svc::dataLocator.get().save["player_data"]["orbs"].as<int>();

	// load player's arsenal
	svc::playerLocator.get().arsenal.loadout.clear();

	return lookup::get_map_label.at(101);
}

void DataManager::load_player_params() {

	std::cout << "loading player params ...";
	player_params = dj::Json::from_file((finder.resource_path + "/data/player/physics_params.json").c_str());
	assert(!player_params.is_null());

	svc::playerLocator.get().physics_stats.grav = player_params["physics"]["grav"].as<float>();
	svc::playerLocator.get().physics_stats.ground_fric = player_params["physics"]["ground_fric"].as<float>();
	svc::playerLocator.get().physics_stats.air_fric = player_params["physics"]["air_fric"].as<float>();
	svc::playerLocator.get().physics_stats.x_acc = player_params["physics"]["x_acc"].as<float>();
	svc::playerLocator.get().physics_stats.air_multiplier = player_params["physics"]["air_multiplier"].as<float>();
	svc::playerLocator.get().physics_stats.jump_velocity = player_params["physics"]["jump_velocity"].as<float>();
	svc::playerLocator.get().physics_stats.jump_release_multiplier = player_params["physics"]["jump_release_multiplier"].as<float>();
	svc::playerLocator.get().physics_stats.hurt_acc = player_params["physics"]["hurt_acc"].as<float>();

	svc::playerLocator.get().physics_stats.maximum_velocity.x = player_params["physics"]["maximum_velocity"]["x"].as<float>();
	svc::playerLocator.get().physics_stats.maximum_velocity.y = player_params["physics"]["maximum_velocity"]["y"].as<float>();

	svc::playerLocator.get().physics_stats.mass = player_params["physics"]["mass"].as<float>();
	svc::playerLocator.get().physics_stats.vertical_dash_multiplier = player_params["physics"]["vertical_dash_multiplier"].as<float>();
	svc::playerLocator.get().physics_stats.dash_speed = player_params["physics"]["dash_speed"].as<float>();
	svc::playerLocator.get().physics_stats.dash_dampen = player_params["physics"]["dash_dampen"].as<float>();
	std::cout << " success!\n";
}

void DataManager::save_player_params() {

	std::cout << "saving player params ...";
	player_params["physics"]["grav"] = svc::playerLocator.get().physics_stats.grav;
	player_params["physics"]["ground_fric"] = svc::playerLocator.get().physics_stats.ground_fric;
	player_params["physics"]["air_fric"] = svc::playerLocator.get().physics_stats.air_fric;
	player_params["physics"]["x_acc"] = svc::playerLocator.get().physics_stats.x_acc;
	player_params["physics"]["air_multiplier"] = svc::playerLocator.get().physics_stats.air_multiplier;
	player_params["physics"]["jump_velocity"] = svc::playerLocator.get().physics_stats.jump_velocity;
	player_params["physics"]["jump_release_multiplier"] = svc::playerLocator.get().physics_stats.jump_release_multiplier;
	player_params["physics"]["hurt_acc"] = svc::playerLocator.get().physics_stats.hurt_acc;

	player_params["physics"]["maximum_velocity"]["x"] = svc::playerLocator.get().physics_stats.maximum_velocity.x;
	player_params["physics"]["maximum_velocity"]["y"] = svc::playerLocator.get().physics_stats.maximum_velocity.y;

	player_params["physics"]["mass"] = svc::playerLocator.get().physics_stats.mass;
	player_params["physics"]["vertical_dash_multiplier"] = svc::playerLocator.get().physics_stats.vertical_dash_multiplier;
	player_params["physics"]["dash_speed"] = svc::playerLocator.get().physics_stats.dash_speed;
	player_params["physics"]["dash_dampen"] = svc::playerLocator.get().physics_stats.dash_dampen;

	player_params.dj::Json::to_file((finder.resource_path + "/data/player/physics_params.json").c_str());
	std::cout << " success!\n";
}

} // namespace data
