
#include "fornani/setup/DataManager.hpp"
#include "fornani/entities/player/Player.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/setup/ControllerMap.hpp"

namespace data {

DataManager::DataManager(automa::ServiceProvider& svc, char** argv) : m_services(&svc) {
	load_data();
}

void DataManager::load_data(std::string in_room) {
	m_services->stopwatch.start();
	std::cout << "> Start Timer...\n";
	auto const& finder = m_services->finder;
	// populate map table
	auto room_path = std::filesystem::path{finder.resource_path()};
	auto room_list = room_path / "level";
	for (auto const& this_region : std::filesystem::recursive_directory_iterator(room_list)) {
		if (!this_region.is_directory()) { continue; }
		for (auto const& this_room : std::filesystem::recursive_directory_iterator(this_region)) {
			if (this_room.path().extension() != ".json") { continue; }
			auto room_data = dj::Json::from_file((this_room.path()).string().c_str());
			if (room_data.is_null()) { continue; }
			auto this_id = room_data["meta"]["room_id"].as<int>();
			auto this_name = this_room.path().filename().string();
			if (is_duplicate_room(this_id)) { continue; }
			map_jsons.push_back(MapData{this_id, room_data});
			
			// cache map layers
			int layer_counter{};
			sf::Vector2<uint32_t> dimensions{};
			dimensions.x = map_jsons.back().metadata["meta"]["dimensions"][0].as<int>();
			dimensions.y = map_jsons.back().metadata["meta"]["dimensions"][1].as<int>();
			std::vector<world::Layer> next{};
			for (int i = 0; i < num_layers; ++i) {
				next.push_back(world::Layer(i, (i == 4), dimensions, map_jsons.back().metadata["tile"]["layers"][layer_counter]));
				++layer_counter;
			}
			map_layers.push_back(next);

			// write to map table
			auto entry = dj::Json{};
			entry["room_id"] = this_id;
			entry["label"] = this_name;
			map_table["rooms"].push_back(entry);
		}
	}
	map_table.dj::Json::to_file((finder.resource_path() + "/data/level/map_table.json").c_str());
	map_table = dj::Json::from_file((finder.resource_path() + "/data/level/map_table.json").c_str());
	assert(!map_table.is_null());
	for (auto const& room : map_table["rooms"].array_view()) {
		auto id = room["room_id"].as<int>();
		if (m_services->tables.get_map_label.contains(id)) { continue; }
		m_services->tables.get_map_label.insert(std::make_pair(id, room["label"].as_string()));
		rooms.push_back(room["room_id"].as<int>());
	}

	auto ctr{0};
	for (auto& file : files) {
		file.id = ctr;
		file.label = "file_" + std::to_string(ctr);
		file.save_data = dj::Json::from_file((finder.resource_path() + "/data/save/file_" + std::to_string(ctr) + ".json").c_str());
		if ((dj::Boolean)file.save_data["status"]["new"].as_bool()) { file.flags.set(fornani::FileFlags::new_file); }
		++ctr;
	}
	blank_file.save_data = dj::Json::from_file((finder.resource_path() + "/data/save/new_game.json").c_str());
	// std::cout << " success!\n";

	// std::cout << "loading json data...";
	weapon = dj::Json::from_file((finder.resource_path() + "/data/weapon/weapon_data.json").c_str());
	assert(!weapon.is_null());
	enemy_weapon = dj::Json::from_file((finder.resource_path() + "/data/weapon/enemy_weapons.json").c_str());
	assert(!enemy_weapon.is_null());
	drop = dj::Json::from_file((finder.resource_path() + "/data/item/drop.json").c_str());
	assert(!drop.is_null());
	particle = dj::Json::from_file((finder.resource_path() + "/data/vfx/particle.json").c_str());
	assert(!particle.is_null());
	sparkler = dj::Json::from_file((finder.resource_path() + "/data/vfx/sparkler.json").c_str());
	assert(!sparkler.is_null());
	npc = dj::Json::from_file((finder.resource_path() + "/data/npc/npc_data.json").c_str());
	assert(!npc.is_null());
	item = dj::Json::from_file((finder.resource_path() + "/data/item/item.json").c_str());
	assert(!item.is_null());
	platform = dj::Json::from_file((finder.resource_path() + "/data/level/platform.json").c_str());
	assert(!platform.is_null());
	cutscene = dj::Json::from_file((finder.resource_path() + "/data/story/cutscenes.json").c_str());
	assert(!cutscene.is_null());
	map_styles = dj::Json::from_file((finder.resource_path() + "/data/level/map_styles.json").c_str());
	assert(!map_styles.is_null());
	action_names = dj::Json::from_file((finder.resource_path() + "/data/gui/action_names.json").c_str());
	assert(!action_names.is_null());

	enemy = dj::Json::from_file((finder.resource_path() + "/data/enemy/enemy_params.json").c_str());
	assert(!enemy.is_null());
	frdog = dj::Json::from_file((finder.resource_path() + "/data/enemy/frdog.json").c_str());
	assert(!frdog.is_null());
	hulmet = dj::Json::from_file((finder.resource_path() + "/data/enemy/hulmet.json").c_str());
	assert(!hulmet.is_null());

	menu = dj::Json::from_file((finder.resource_path() + "/data/gui/menu.json").c_str());
	assert(!menu.is_null());
	background = dj::Json::from_file((finder.resource_path() + "/data/level/background_behaviors.json").c_str());
	assert(!background.is_null());

	// load item labels
	for (auto const& entry : item.object_view()) { m_services->tables.item_labels.insert({entry.second["index"].as<int>(), entry.first}); }

	// load marketplace
	for (auto const& entry : npc.object_view()) {
		if (!entry.second["vendor"]) { continue; }
		marketplace.insert({entry.second["vendor"]["id"].as<int>(), npc::Vendor()});
		auto& vendor = marketplace.at(entry.second["vendor"]["id"].as<int>());
		vendor.set_upcharge(entry.second["vendor"]["upcharge"].as<float>());
		for (auto& item : entry.second["vendor"]["common_items"].array_view()) { vendor.common_items.push_back(item.as<int>()); }
		for (auto& item : entry.second["vendor"]["uncommon_items"].array_view()) { vendor.uncommon_items.push_back(item.as<int>()); }
		for (auto& item : entry.second["vendor"]["rare_items"].array_view()) { vendor.rare_items.push_back(item.as<int>()); }
	}
	m_services->stopwatch.stop();
	m_services->stopwatch.print_time();
	m_services->stopwatch.start();
}

void DataManager::save_progress(player::Player& player, int save_point_id) {
	auto& save = files.at(current_save).save_data;
	files.at(current_save).write();
	// set file data based on player state
	save["player_data"]["max_hp"] = player.health.get_max();
	save["player_data"]["hp"] = player.health.get_hp();
	save["player_data"]["orbs"] = player.wallet.get_balance();
	save["player_data"]["position"]["x"] = player.collider.physics.position.x;
	save["player_data"]["position"]["y"] = player.collider.physics.position.y;

	// create empty json array
	constexpr auto empty_array = R"([])";
	auto const wipe = dj::Json::parse(empty_array);

	// write marketplace status
	save["marketplace"] = wipe;
	for (auto& vendor : marketplace) {
		auto out_vendor = wipe;
		for (auto& item : vendor.second.inventory.items) { out_vendor.push_back(item.get_id()); }
		save["marketplace"].push_back(out_vendor);
	}

	// write opened chests and doors
	save["map_data"]["world_time"]["hours"] = m_services->world_clock.get_hours();
	save["map_data"]["world_time"]["minutes"] = m_services->world_clock.get_minutes();
	save["piggybacker"] = m_services->player_dat.piggy_id;
	save["npc_locations"] = wipe;
	save["map_data"]["fallen_enemies"] = wipe;
	save["discovered_rooms"] = wipe;
	save["unlocked_doors"] = wipe;
	save["opened_chests"] = wipe;
	save["activated_switches"] = wipe;
	save["destroyed_blocks"] = wipe;
	save["destroyed_inspectables"] = wipe;
	for (auto& entry : save["quest_progressions"].array_view()) { entry = wipe; }
	save["quest_progressions"] = wipe;
	for (auto& location : npc_locations) {
		auto entry = wipe;
		entry.push_back(location.first);
		entry.push_back(location.second);
		save["npc_locations"].push_back(entry);
	}
	for (auto& enemy : fallen_enemies) {
		auto entry = wipe;
		entry.push_back(enemy.code.first);
		entry.push_back(enemy.code.second);
		entry.push_back(enemy.respawn_distance);
		entry.push_back(static_cast<int>(enemy.permanent));
		save["map_data"]["fallen_enemies"].push_back(entry);
	}
	for (auto& room : discovered_rooms) { save["discovered_rooms"].push_back(room); }
	for (auto& door : unlocked_doors) { save["unlocked_doors"].push_back(door); }
	for (auto& chest : opened_chests) { save["opened_chests"].push_back(chest); }
	for (auto& s : activated_switches) { save["activated_switches"].push_back(s); }
	for (auto& block : destroyed_blocks) { save["destroyed_blocks"].push_back(block); }
	for (auto& i : destroyed_inspectables) { save["destroyed_inspectables"].push_back(i); }
	for (auto& q : quest_progressions) {
		auto out_quest = wipe;
		out_quest.push_back(q.type);
		out_quest.push_back(q.id);
		out_quest.push_back(q.source_id);
		out_quest.push_back(q.amount);
		out_quest.push_back(q.hard_set);
		save["quest_progressions"].push_back(out_quest);
	}

	save["tutorial"]["jump"] = dj::Boolean{player.tutorial.flags.test(text::TutorialFlags::jump)};
	save["tutorial"]["shoot"] = dj::Boolean{player.tutorial.flags.test(text::TutorialFlags::shoot)};
	save["tutorial"]["sprint"] = dj::Boolean{player.tutorial.flags.test(text::TutorialFlags::sprint)};
	save["tutorial"]["map"] = dj::Boolean{player.tutorial.flags.test(text::TutorialFlags::map)};
	save["tutorial"]["inventory"] = dj::Boolean{player.tutorial.flags.test(text::TutorialFlags::inventory)};
	save["tutorial"]["state"] = static_cast<int>(player.tutorial.current_state);
	save["tutorial"]["closed"] = dj::Boolean{player.tutorial.closed()};
	save["tutorial"]["on"] = dj::Boolean{player.tutorial.on()};

	// save arsenal
	save["player_data"]["arsenal"] = wipe;
	save["player_data"]["hotbar"] = wipe;
	// push player arsenal
	if (player.arsenal) {
		for (auto& gun : player.arsenal.value().get_loadout()) {
			int this_id = gun->get_id();
			save["player_data"]["arsenal"].push_back(this_id);
		}
		if (player.hotbar) {
			for (auto& id : player.hotbar.value().get_ids()) { save["player_data"]["hotbar"].push_back(id); }
			save["player_data"]["equipped_gun"] = player.hotbar.value().get_id();
		}
	}

	// wardrobe
	save["player_data"]["wardrobe"]["hairstyle"] = player.catalog.categories.wardrobe.get_variant(player::ApparelType::hairstyle);
	save["player_data"]["wardrobe"]["shirt"] = player.catalog.categories.wardrobe.get_variant(player::ApparelType::shirt);
	save["player_data"]["wardrobe"]["pants"] = player.catalog.categories.wardrobe.get_variant(player::ApparelType::pants);

	// items and abilities
	save["player_data"]["abilities"] = wipe;
	save["player_data"]["items"] = wipe;
	if (player.catalog.categories.abilities.has_ability(player::Abilities::dash)) { save["player_data"]["abilities"].push_back("dash"); }
	if (player.catalog.categories.abilities.has_ability(player::Abilities::wall_slide)) { save["player_data"]["abilities"].push_back("wallslide"); }
	if (player.catalog.categories.abilities.has_ability(player::Abilities::double_jump)) { save["player_data"]["abilities"].push_back("doublejump"); }
	for (auto& item : player.catalog.categories.inventory.items) {
		dj::Json this_item{};
		this_item["id"] = item.get_id();
		this_item["quantity"] = item.get_quantity();
		save["player_data"]["items"].push_back(this_item);
	}

	save["save_point_id"] = save_point_id;

	// stat tracker
	auto& out_stat = save["player_data"]["stats"];
	auto const& s = m_services->stats;
	out_stat["death_count"] = s.player.death_count.get_count();
	out_stat["bullets_fired"] = s.player.bullets_fired.get_count();
	out_stat["guns_collected"] = s.player.guns_collected.get_count();
	out_stat["items_collected"] = s.player.items_collected.get_count();
	out_stat["orbs_collected"] = s.treasure.total_orbs_collected.get_count();
	out_stat["blue_orbs"] = s.treasure.blue_orbs.get_count();
	out_stat["highest_indicator_amount"] = s.treasure.highest_indicator_amount.get_count();
	out_stat["enemies_killed"] = s.enemy.enemies_killed.get_count();
	out_stat["rooms_discovered"] = s.world.rooms_discovered.get_count();
	out_stat["seconds_played"] = m_services->ticker.in_game_seconds_passed.count();
	out_stat["time_trials"]["bryns_gun"] = s.time_trials.bryns_gun;

	save.dj::Json::to_file((m_services->finder.resource_path() + "/data/save/file_" + std::to_string(current_save) + ".json").c_str());
}

void DataManager::save_settings() {
	settings["auto_sprint"] = dj::Boolean{m_services->controller_map.is_autosprint_enabled()};
	settings["tutorial"] = dj::Boolean{m_services->tutorial()};
	settings["gamepad"] = dj::Boolean{m_services->controller_map.is_gamepad_input_enabled()};
	settings["music_volume"] = m_services->music.volume.multiplier;
	settings["fullscreen"] = dj::Boolean{m_services->fullscreen()};
	settings.dj::Json::to_file((m_services->finder.resource_path() + "/data/config/settings.json").c_str());
}

int DataManager::load_progress(player::Player& player, int const file, bool state_switch, bool from_menu) {

	current_save = file;
	auto const& save = files.at(file).save_data;
	assert(!save.is_null());

	// marketplace
	for (auto& vendor : marketplace) {
		vendor.second.inventory.items.clear();
		for (auto& v : save["marketplace"].array_view()) {
			for (auto& id : v.array_view()) { vendor.second.inventory.add_item(*m_services, id.as<int>(), 1); }
		}
	}

	m_services->quest = {};
	discovered_rooms.clear();
	unlocked_doors.clear();
	opened_chests.clear();
	destroyed_blocks.clear();
	activated_switches.clear();
	destroyed_inspectables.clear();
	quest_progressions.clear();
	npc_locations.clear();
	fallen_enemies.clear();

	m_services->world_clock.set_time(save["map_data"]["world_time"]["hours"].as<int>(), save["map_data"]["world_time"]["minutes"].as<int>());
	for (auto& room : save["discovered_rooms"].array_view()) { discovered_rooms.push_back(room.as<int>()); }
	for (auto& door : save["unlocked_doors"].array_view()) { unlocked_doors.push_back(door.as<int>()); }
	for (auto& chest : save["opened_chests"].array_view()) { opened_chests.push_back(chest.as<int>()); }
	for (auto& s : save["activated_switches"].array_view()) { activated_switches.push_back(s.as<int>()); }
	for (auto& block : save["destroyed_blocks"].array_view()) { destroyed_blocks.push_back(block.as<int>()); }
	for (auto& inspectable : save["destroyed_inspectables"].array_view()) { destroyed_inspectables.push_back(inspectable.as_string().data()); }
	for (auto& q : save["quest_progressions"].array_view()) {
		auto type = q[0].as<int>();
		auto id = q[1].as<int>();
		auto srcid = q[2].as<int>();
		auto amt = q[3].as<int>();
		auto hard = q[4].as<int>();
		quest_progressions.push_back(util::QuestKey{type, id, srcid, amt, hard});
		m_services->quest.process(*m_services, quest_progressions.back());
	}
	for (auto& location : save["npc_locations"].array_view()) { npc_locations.insert({location[0].as<int>(), location[1].as<int>()}); }
	for (auto& enemy : save["map_data"]["fallen_enemies"].array_view()) { fallen_enemies.push_back({std::make_pair(enemy[0].as<int>(), enemy[1].as<int>()), enemy[2].as<int>(), static_cast<bool>(enemy[3].as<int>())}); };
	player.piggybacker = {};
	m_services->player_dat.set_piggy_id(save["piggybacker"].as<int>());
	m_services->player_dat.drop_piggy = false;

	player.tutorial.flags = {};
	if (save["tutorial"]["jump"].as_bool()) { player.tutorial.flags.set(text::TutorialFlags::jump); }
	if (save["tutorial"]["shoot"].as_bool()) { player.tutorial.flags.set(text::TutorialFlags::shoot); }
	if (save["tutorial"]["sprint"].as_bool()) { player.tutorial.flags.set(text::TutorialFlags::sprint); }
	if (save["tutorial"]["map"].as_bool()) { player.tutorial.flags.set(text::TutorialFlags::map); }
	if (save["tutorial"]["inventory"].as_bool()) { player.tutorial.flags.set(text::TutorialFlags::inventory); }
	player.tutorial.current_state = static_cast<text::TutorialFlags>(save["tutorial"]["state"].as<int>());
	if (save["tutorial"]["closed"].as_bool()) { player.tutorial.close_for_good(); }
	player.cooldowns.tutorial.start();
	player.tutorial.turn_off();
	if (!m_services->tutorial()) { player.tutorial.close_for_good(); }

	int save_pt_id = save["save_point_id"].as<int>();
	int room_id = save_pt_id;
	m_services->state_controller.save_point_id = save_pt_id;

	// set player data based on save file
	player.health.set_max(save["player_data"]["max_hp"].as<float>());
	player.health.set_hp(save["player_data"]["hp"].as<float>());
	player.wallet.set_balance(save["player_data"]["orbs"].as<int>());

	// load player's arsenal
	player.arsenal = {};
	player.hotbar = {};
	for (auto& gun_id : save["player_data"]["arsenal"].array_view()) { player.push_to_loadout(gun_id.as<int>(), true); }
	if (!save["player_data"]["hotbar"].array_view().empty()) {
		if (!player.hotbar) { player.hotbar = arms::Hotbar(1); }
		if (player.hotbar) {
			for (auto& gun_id : save["player_data"]["hotbar"].array_view()) { player.hotbar.value().add(gun_id.as<int>()); }
			auto equipped_gun = save["player_data"]["equipped_gun"].as<int>();
			player.hotbar.value().set_selection(equipped_gun);
		}
	}

	// load items and abilities
	player.catalog.categories.abilities.clear();
	player.catalog.categories.inventory.clear();
	for (auto& ability : save["player_data"]["abilities"].array_view()) { player.catalog.categories.abilities.give_ability(ability.as<int>()); }
	for (auto& item : save["player_data"]["items"].array_view()) { player.catalog.categories.inventory.add_item(*m_services, item["id"].as<int>(), item["quantity"].as<int>()); }

	// wardrobe
	auto& wardrobe = player.catalog.categories.wardrobe;
	auto hairstyle = save["player_data"]["wardrobe"]["hairstyle"].as<int>();
	auto shirt = save["player_data"]["wardrobe"]["shirt"].as<int>();
	auto pants = save["player_data"]["wardrobe"]["pants"].as<int>();
	hairstyle > 0 ? player.equip_item(player::ApparelType::hairstyle, hairstyle + 80) : wardrobe.unequip(player::ApparelType::hairstyle);
	shirt > 0 ? player.equip_item(player::ApparelType::shirt, shirt + 80) : wardrobe.unequip(player::ApparelType::shirt);
	pants > 0 ? player.equip_item(player::ApparelType::pants, pants + 80) : wardrobe.unequip(player::ApparelType::pants);
	player.catalog.categories.wardrobe.update(player.texture_updater);

	// stat tracker
	auto& s = m_services->stats;
	auto deaths = s.player.death_count.get_count();
	s = {};
	if (!from_menu) { s.player.death_count.set(deaths); }
	auto const& in_stat = save["player_data"]["stats"];
	if (from_menu) { s.player.death_count.set(in_stat["death_count"].as<int>()); }
	s.player.bullets_fired.set(in_stat["bullets_fired"].as<int>());
	s.player.guns_collected.set(in_stat["guns_collected"].as<int>());
	s.player.items_collected.set(in_stat["items_collected"].as<int>());
	s.treasure.total_orbs_collected.set(in_stat["orbs_collected"].as<int>());
	s.treasure.blue_orbs.set(in_stat["blue_orbs"].as<int>());
	s.treasure.highest_indicator_amount.set(in_stat["highest_indicator_amount"].as<int>());
	s.enemy.enemies_killed.set(in_stat["enemies_killed"].as<int>());
	s.world.rooms_discovered.set(in_stat["rooms_discovered"].as<int>());
	s.time_trials.bryns_gun = in_stat["time_trials"]["bryns_gun"].as<float>();
	m_services->ticker.set_time(m_services->stats.float_to_seconds(in_stat["seconds_played"].as<float>()));
	if (files.at(file).flags.test(fornani::FileFlags::new_file)) { s.player.death_count.set(0); }

	return room_id;
}

void DataManager::load_settings() {
	settings = dj::Json::from_file((m_services->finder.resource_path() + "/data/config/settings.json").c_str());
	assert(!settings.is_null());
	m_services->controller_map.enable_autosprint(settings["auto_sprint"].as_bool().value);
	m_services->set_tutorial(settings["tutorial"].as_bool().value);
	m_services->controller_map.enable_gamepad_input(settings["gamepad"].as_bool().value);
	m_services->music.volume.multiplier = settings["music_volume"].as<float>();
	m_services->set_fullscreen(settings["fullscreen"].as_bool().value);
	std::cout << "Settings set.\n";
}

void DataManager::delete_file(int index) {
	if (index >= files.size()) { return; }
	files.at(index).save_data = blank_file.save_data;
	files.at(index).flags.set(fornani::FileFlags::new_file);
	files.at(index).save_data.dj::Json::to_file((m_services->finder.resource_path() + "/data/save/file_" + std::to_string(current_save) + ".json").c_str());
}

void DataManager::write_death_count(player::Player& player) {
	auto& save = files.at(current_save).save_data;
	auto& out_stat = save["player_data"]["stats"];
	auto const& s = m_services->stats;
	out_stat["death_count"] = s.player.death_count.get_count();
	save.dj::Json::to_file((m_services->finder.resource_path() + "/data/save/file_" + std::to_string(current_save) + ".json").c_str());
}

std::string_view DataManager::load_blank_save(player::Player& player, bool state_switch) {

	auto const& save = blank_file.save_data;
	assert(!save.is_null());

	// set player data based on save file
	player.health.set_max(save["player_data"]["max_hp"].as<float>());
	player.health.set_hp(save["player_data"]["hp"].as<float>());
	player.wallet.set_balance(save["player_data"]["orbs"].as<int>());

	// load player's arsenal
	player.arsenal = {};

	return m_services->tables.get_map_label.at(1);
}

void DataManager::load_player_params(player::Player& player) {

	// std::cout << "loading player params ...";
	player_params = dj::Json::from_file((m_services->finder.resource_path() + "/data/player/physics_params.json").c_str());
	assert(!player_params.is_null());

	player.physics_stats.grav = player_params["physics"]["grav"].as<float>();
	player.physics_stats.ground_fric = player_params["physics"]["ground_fric"].as<float>();
	player.physics_stats.air_fric = player_params["physics"]["air_fric"].as<float>();
	player.physics_stats.x_acc = player_params["physics"]["x_acc"].as<float>();
	player.physics_stats.air_multiplier = player_params["physics"]["air_multiplier"].as<float>();
	player.physics_stats.sprint_multiplier = player_params["physics"]["sprint_multiplier"].as<float>();
	player.physics_stats.jump_velocity = player_params["physics"]["jump_velocity"].as<float>();
	player.physics_stats.jump_release_multiplier = player_params["physics"]["jump_release_multiplier"].as<float>();
	player.physics_stats.hurt_acc = player_params["physics"]["hurt_acc"].as<float>();

	player.physics_stats.maximum_velocity.x = player_params["physics"]["maximum_velocity"]["x"].as<float>();
	player.physics_stats.maximum_velocity.y = player_params["physics"]["maximum_velocity"]["y"].as<float>();

	player.physics_stats.mass = player_params["physics"]["mass"].as<float>();
	player.physics_stats.vertical_dash_multiplier = player_params["physics"]["vertical_dash_multiplier"].as<float>();
	player.physics_stats.dash_speed = player_params["physics"]["dash_speed"].as<float>();
	player.physics_stats.dash_dampen = player_params["physics"]["dash_dampen"].as<float>();
	player.physics_stats.wallslide_speed = player_params["physics"]["wallslide_speed"].as<float>();
	// std::cout << " success!\n";
}

void DataManager::save_player_params(player::Player& player) {
	auto const& finder = m_services->finder;
	// std::cout << "saving player params ...";
	player_params["physics"]["grav"] = player.physics_stats.grav;
	player_params["physics"]["ground_fric"] = player.physics_stats.ground_fric;
	player_params["physics"]["air_fric"] = player.physics_stats.air_fric;
	player_params["physics"]["x_acc"] = player.physics_stats.x_acc;
	player_params["physics"]["air_multiplier"] = player.physics_stats.air_multiplier;
	player_params["physics"]["sprint_multiplier"] = player.physics_stats.sprint_multiplier;
	player_params["physics"]["jump_velocity"] = player.physics_stats.jump_velocity;
	player_params["physics"]["jump_release_multiplier"] = player.physics_stats.jump_release_multiplier;
	player_params["physics"]["hurt_acc"] = player.physics_stats.hurt_acc;

	player_params["physics"]["maximum_velocity"]["x"] = player.physics_stats.maximum_velocity.x;
	player_params["physics"]["maximum_velocity"]["y"] = player.physics_stats.maximum_velocity.y;

	player_params["physics"]["mass"] = player.physics_stats.mass;
	player_params["physics"]["vertical_dash_multiplier"] = player.physics_stats.vertical_dash_multiplier;
	player_params["physics"]["dash_speed"] = player.physics_stats.dash_speed;
	player_params["physics"]["dash_dampen"] = player.physics_stats.dash_dampen;
	player_params["physics"]["wallslide_speed"] = player.physics_stats.wallslide_speed;

	player_params.dj::Json::to_file((finder.resource_path() + "/data/player/physics_params.json").c_str());
	// std::cout << " success!\n";
}

void DataManager::open_chest(int id) { opened_chests.push_back(id); }

void DataManager::unlock_door(int id) { unlocked_doors.push_back(id); }

void DataManager::activate_switch(int id) {
	if (!switch_is_activated(id)) { activated_switches.push_back(id); }
}

void DataManager::destroy_block(int id) {
	if (std::find(destroyed_blocks.begin(), destroyed_blocks.end(), id) != destroyed_blocks.end()) { return; }
	destroyed_blocks.push_back(id);
}

void DataManager::destroy_inspectable(std::string_view id) { destroyed_inspectables.push_back(id.data()); }

void DataManager::push_quest(util::QuestKey key) {
	for (auto& entry : quest_progressions) {
		if (entry == key) { return; }
	}
	quest_progressions.push_back(key);
}

void DataManager::set_npc_location(int npc_id, int room_id) {
	if (room_id < 0) { return; }
	if (!npc_locations.contains(npc_id)) { npc_locations.insert({npc_id, room_id}); }
	npc_locations.at(npc_id) = room_id;
}

void DataManager::kill_enemy(int room_id, int id, int distance, bool permanent) {
	for (auto& e : fallen_enemies) {
		if (e.code.first == room_id && e.code.second) { return; }
	}
	fallen_enemies.push_back({{room_id, id}, distance, permanent});
}

void DataManager::respawn_enemy(int room_id, int id) {
	std::erase_if(fallen_enemies, [room_id, id](auto const& i) { return i.code.first == room_id && i.code.second == id && !i.permanent; });
}

void DataManager::respawn_enemies(int room_id, int distance) {
	std::erase_if(fallen_enemies, [room_id, distance](auto const& i) { return i.code.first == room_id && i.respawn_distance < distance && !i.permanent; });
}

void DataManager::respawn_all() {
	std::erase_if(fallen_enemies, [](auto const& i) { return !i.permanent; });
}

bool data::DataManager::is_duplicate_room(int id) const {
	for (auto& json : map_jsons) {
		if (json.id == id) { return true; }
	}
	return false;
}

bool DataManager::door_is_unlocked(int id) const {
	for (auto& door : unlocked_doors) {
		if (door == id) { return true; }
	}
	return false;
}

bool DataManager::chest_is_open(int id) const {
	for (auto& chest : opened_chests) {
		if (chest == id) { return true; }
	}
	return false;
}

bool DataManager::switch_is_activated(int id) const {
	for (auto& s : activated_switches) {
		if (s == id) { return true; }
	}
	return false;
}

bool DataManager::block_is_destroyed(int id) const {
	for (auto& b : destroyed_blocks) {
		if (b == id) { return true; }
	}
	return false;
}

bool DataManager::inspectable_is_destroyed(std::string_view id) const {
	for (auto& i : destroyed_inspectables) {
		if (i == id) { return true; }
	}
	return false;
}

bool DataManager::room_discovered(int id) const {
	for (auto& room : discovered_rooms) {
		if (id == room) { return true; }
	}
	return false;
}

bool DataManager::enemy_is_fallen(int room_id, int id) const {
	for (auto& enemy : fallen_enemies) {
		if (enemy.code.first == room_id && enemy.code.second == id) { return true; }
	}
	return false;
}

auto get_action_by_string(std::string_view id) -> config::DigitalAction {
	static std::unordered_map<std::string_view, config::DigitalAction> const map = {
		{"platformer_left", config::DigitalAction::platformer_left},
		{"platformer_right", config::DigitalAction::platformer_right},
		{"platformer_up", config::DigitalAction::platformer_up},
		{"platformer_down", config::DigitalAction::platformer_down},
		{"platformer_jump", config::DigitalAction::platformer_jump},
		{"platformer_shoot", config::DigitalAction::platformer_shoot},
		{"platformer_sprint", config::DigitalAction::platformer_sprint},
		{"platformer_shield", config::DigitalAction::platformer_shield},
		{"platformer_inspect", config::DigitalAction::platformer_inspect},
		{"platformer_arms_switch_left", config::DigitalAction::platformer_arms_switch_left},
		{"platformer_arms_switch_right", config::DigitalAction::platformer_arms_switch_right},
		{"platformer_open_inventory", config::DigitalAction::platformer_open_inventory},
		{"platformer_open_map", config::DigitalAction::platformer_open_map},
		{"platformer_toggle_pause", config::DigitalAction::platformer_toggle_pause},
		{"inventory_open_map", config::DigitalAction::inventory_open_map},
		{"inventory_close", config::DigitalAction::inventory_close},
		{"map_open_inventory", config::DigitalAction::map_open_inventory},
		{"map_close", config::DigitalAction::map_close},
		{"menu_left", config::DigitalAction::menu_left},
		{"menu_right", config::DigitalAction::menu_right},
		{"menu_up", config::DigitalAction::menu_up},
		{"menu_down", config::DigitalAction::menu_down},
		{"menu_select", config::DigitalAction::menu_select},
		{"menu_switch_left", config::DigitalAction::menu_switch_left},
		{"menu_switch_right", config::DigitalAction::menu_switch_right},
		{"menu_cancel", config::DigitalAction::menu_cancel},
	};

	return map.at(id);
}

void DataManager::load_controls(config::ControllerMap& controller) {
	// XXX change controls json when keybinds get modified
	controls = dj::Json::from_file((m_services->finder.resource_path() + "/data/config/control_map.json").c_str());
	assert(!controls.is_null());
	assert(controls.contains("controls") && controls["controls"].is_object());

	for (auto const& [key, item] : controls["controls"].object_view()) {
		assert(item.is_object());
		if (item.contains("primary_key")) { controller.set_primary_keyboard_binding(get_action_by_string(key), controller.string_to_key(item["primary_key"].as_string())); }
	}
}

void DataManager::save_controls(config::ControllerMap& controller) { controls.dj::Json::to_file((m_services->finder.resource_path() + "/data/config/control_map.json").c_str()); }

void DataManager::reset_controls() { controls = dj::Json::from_file((m_services->finder.resource_path() + "/data/config/defaults.json").c_str()); }

int DataManager::get_room_index(int id) {
	auto ctr{0};
	for (auto& room : rooms) {
		if (room == id) { return ctr; }
		++ctr;
	}
	return ctr;
}

int DataManager::get_npc_location(int npc_id) {
	if (!npc_locations.contains(npc_id)) { return 0; }
	return npc_locations.at(npc_id);
}

std::vector<world::Layer>& DataManager::get_layers(int id) { return map_layers.at(get_room_index(id)); }

} // namespace data
