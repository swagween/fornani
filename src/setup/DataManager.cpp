
#pragma once

#include "DataManager.hpp"
#include "MapLookups.hpp"
#include "../service/ServiceProvider.hpp"
#include "../entities/player/Player.hpp"

namespace data {

DataManager::DataManager(automa::ServiceProvider& svc) : m_services(&svc) {}

void DataManager::load_data() {

	std::cout << "loading json data...";
	weapon = dj::Json::from_file((finder.resource_path + "/data/weapon/weapon_data.json").c_str());
	assert(!weapon.is_null());
	drop = dj::Json::from_file((finder.resource_path + "/data/item/drop.json").c_str());
	assert(!drop.is_null());
	particle = dj::Json::from_file((finder.resource_path + "/data/vfx/particle.json").c_str());
	assert(!particle.is_null());
	sparkler = dj::Json::from_file((finder.resource_path + "/data/vfx/sparkler.json").c_str());
	assert(!sparkler.is_null());
	npc = dj::Json::from_file((finder.resource_path + "/data/npc/npc_data.json").c_str());
	assert(!npc.is_null());
	item = dj::Json::from_file((finder.resource_path + "/data/item/item.json").c_str());
	assert(!item.is_null());
	map_styles = dj::Json::from_file((finder.resource_path + "/data/level/map_styles.json").c_str());
	assert(!map_styles.is_null());

	enemy = dj::Json::from_file((finder.resource_path + "/data/enemy/enemy_params.json").c_str());
	assert(!enemy.is_null());
	frdog = dj::Json::from_file((finder.resource_path + "/data/enemy/frdog.json").c_str());
	assert(!frdog.is_null());
	hulmet = dj::Json::from_file((finder.resource_path + "/data/enemy/hulmet.json").c_str());
	assert(!hulmet.is_null());

	map_table = dj::Json::from_file((finder.resource_path + "/data/level/map_table.json").c_str());
	assert(!map_table.is_null());
	for (auto const& room : map_table["rooms"].array_view()) { lookup::get_map_label.insert(std::make_pair(room["room_id"].as<int>(), room["label"].as_string())); }

	background = dj::Json::from_file((finder.resource_path + "/data/level/background_behaviors.json").c_str());
	assert(!background.is_null());
	std::cout << " success!\n";
}

void DataManager::save_progress(player::Player& player, int save_point_id) {

	// set file data based on player state
	save["player_data"]["max_hp"] = player.health.get_max();
	save["player_data"]["hp"] = player.health.get_hp();
	save["player_data"]["orbs"] = player.player_stats.orbs;
	save["player_data"]["position"]["x"] = player.collider.physics.position.x;
	save["player_data"]["position"]["y"] = player.collider.physics.position.y;

	// save arsenal
	// wipe it first
	constexpr auto empty_array = R"([])";
	auto const wipe = dj::Json::parse(empty_array);
	save["player_data"]["arsenal"] = wipe;
	// push player arsenal
	for (auto& gun : player.arsenal.loadout) {
		int this_id = gun->get_id();
		save["player_data"]["arsenal"].push_back(this_id);
	}
	save["player_data"]["equipped_gun"] = player.arsenal.get_index();

	//items and abilities
	save["player_data"]["abilities"] = wipe;
	save["player_data"]["items"] = wipe;
	if (player.catalog.categories.abilities.has_ability(player::Abilities::dash)) { save["player_data"]["abilities"].push_back("dash"); }
	for(auto& item : player.catalog.categories.inventory.items) {
		dj::Json this_item{};
		this_item["id"] = item.get_id();
		this_item["quantity"] = item.get_quantity();
		save["player_data"]["items"].push_back(this_item);
	}

	save["save_point_id"] = save_point_id;

	save.dj::Json::to_file((finder.resource_path + "/data/save/file_" + std::to_string(current_save) + ".json").c_str());
}

std::string_view DataManager::load_progress(player::Player& player, int const file, bool state_switch) {

	current_save = file;

	save = dj::Json::from_file((finder.resource_path + "/data/save/file_" + std::to_string(file) + ".json").c_str());
	assert(!save.is_null());

	int save_pt_id = save["save_point_id"].as<int>();
	int room_id = lookup::save_point_to_room_id.at(save_pt_id);

	// set player data based on save file
	player.health.set_max(save["player_data"]["max_hp"].as<int>());
	player.health.set_hp(save["player_data"]["hp"].as<int>());
	player.player_stats.orbs = save["player_data"]["orbs"].as<int>();

	// load player's arsenal
	player.arsenal.loadout.clear();
	for (auto& gun_id : save["player_data"]["arsenal"].array_view()) {
		player.arsenal.push_to_loadout(gun_id.as<int>());
	}
	if (!player.arsenal.loadout.empty()) {
		auto equipped_gun = save["player_data"]["equipped_gun"].as<int>();
		player.arsenal.set_index(equipped_gun);
	}

	// load items and abilities
	player.catalog.categories.abilities.clear();
	player.catalog.categories.inventory.clear();
	for (auto& ability : save["player_data"]["abilities"].array_view()) { player.catalog.categories.abilities.give_ability(ability.as_string()); }
	for (auto& item : save["player_data"]["items"].array_view()) { player.catalog.categories.inventory.add_item(*m_services, item["id"].as<int>(), item["quantity"].as<int>()); }

	//reset some things that might be lingering
	player.arsenal.extant_projectile_instances = {};

	return lookup::get_map_label.at(room_id);
}

std::string_view DataManager::load_blank_save(player::Player& player, bool state_switch) {

	save = dj::Json::from_file((finder.resource_path + "/data/save/new_game.json").c_str());
	assert(!save.is_null());

	// set player data based on save file
	player.health.set_max(save["player_data"]["max_hp"].as<int>());
	player.health.set_hp(save["player_data"]["hp"].as<int>());
	player.player_stats.orbs = save["player_data"]["orbs"].as<int>();

	// load player's arsenal
	player.arsenal.loadout.clear();

	return lookup::get_map_label.at(100);
}

void DataManager::load_player_params(player::Player& player) {

	std::cout << "loading player params ...";
	player_params = dj::Json::from_file((finder.resource_path + "/data/player/physics_params.json").c_str());
	assert(!player_params.is_null());

	player.physics_stats.grav = player_params["physics"]["grav"].as<float>();
	player.physics_stats.ground_fric = player_params["physics"]["ground_fric"].as<float>();
	player.physics_stats.air_fric = player_params["physics"]["air_fric"].as<float>();
	player.physics_stats.x_acc = player_params["physics"]["x_acc"].as<float>();
	player.physics_stats.air_multiplier = player_params["physics"]["air_multiplier"].as<float>();
	player.physics_stats.jump_velocity = player_params["physics"]["jump_velocity"].as<float>();
	player.physics_stats.jump_release_multiplier = player_params["physics"]["jump_release_multiplier"].as<float>();
	player.physics_stats.hurt_acc = player_params["physics"]["hurt_acc"].as<float>();

	player.physics_stats.maximum_velocity.x = player_params["physics"]["maximum_velocity"]["x"].as<float>();
	player.physics_stats.maximum_velocity.y = player_params["physics"]["maximum_velocity"]["y"].as<float>();

	player.physics_stats.mass = player_params["physics"]["mass"].as<float>();
	player.physics_stats.vertical_dash_multiplier = player_params["physics"]["vertical_dash_multiplier"].as<float>();
	player.physics_stats.dash_speed = player_params["physics"]["dash_speed"].as<float>();
	player.physics_stats.dash_dampen = player_params["physics"]["dash_dampen"].as<float>();
	std::cout << " success!\n";
}

void DataManager::save_player_params(player::Player& player) {

	std::cout << "saving player params ...";
	player_params["physics"]["grav"] = player.physics_stats.grav;
	player_params["physics"]["ground_fric"] = player.physics_stats.ground_fric;
	player_params["physics"]["air_fric"] = player.physics_stats.air_fric;
	player_params["physics"]["x_acc"] = player.physics_stats.x_acc;
	player_params["physics"]["air_multiplier"] = player.physics_stats.air_multiplier;
	player_params["physics"]["jump_velocity"] = player.physics_stats.jump_velocity;
	player_params["physics"]["jump_release_multiplier"] = player.physics_stats.jump_release_multiplier;
	player_params["physics"]["hurt_acc"] = player.physics_stats.hurt_acc;

	player_params["physics"]["maximum_velocity"]["x"] = player.physics_stats.maximum_velocity.x;
	player_params["physics"]["maximum_velocity"]["y"] = player.physics_stats.maximum_velocity.y;

	player_params["physics"]["mass"] = player.physics_stats.mass;
	player_params["physics"]["vertical_dash_multiplier"] = player.physics_stats.vertical_dash_multiplier;
	player_params["physics"]["dash_speed"] = player.physics_stats.dash_speed;
	player_params["physics"]["dash_dampen"] = player.physics_stats.dash_dampen;

	player_params.dj::Json::to_file((finder.resource_path + "/data/player/physics_params.json").c_str());
	std::cout << " success!\n";
}

} // namespace data
