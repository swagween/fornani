
#include "fornani/setup/DataManager.hpp"
#include "fornani/entities/player/Player.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/setup/ControllerMap.hpp"

namespace fornani::data {

DataManager::DataManager(automa::ServiceProvider& svc) : m_services(&svc), minimap{svc} { load_data(); }

void DataManager::load_data(std::string in_room) {
	m_services->stopwatch.start();
	NANI_LOG_INFO(m_logger, "Data loading started.");
	auto const& finder = m_services->finder;

	// load audio library
	auto audio_path = std::filesystem::path{finder.resource_path()} / "audio";
	auto song_list = audio_path / "songs";
	auto ambience_list = audio_path / "ambience";
	constexpr auto text = R"({"songs": [], "ambience": []})";
	auto result = dj::Json::parse(text);
	audio_library = result.value();
	assert(!audio_library.is_null());
	// populate library json
	for (auto const& song : std::filesystem::recursive_directory_iterator(song_list)) {
		if (song.path().extension() != ".xm") { continue; }
		audio_library["music"].push_back(song.path().stem().string());
	}
	for (auto const& library : std::filesystem::recursive_directory_iterator(ambience_list)) {
		if (!library.is_directory()) { continue; }
		audio_library["ambience"].push_back(library.path().filename().string());
	}

	auto room_path = std::filesystem::path{finder.resource_path()};
	auto room_list = room_path / "level";
	auto template_list = room_path / "data" / "level" / "templates";
	// populate map templates

	for (auto const& this_biome : std::filesystem::recursive_directory_iterator(template_list)) {
		if (this_biome.path().extension() != ".json") {
			NANI_LOG_ERROR(m_logger, "Found a template with an extension other than \".json\": {}.", this_biome.path().filename().string());
			continue;
		}
		auto template_data_result = dj::Json::from_file(this_biome.path().string());
		if (!template_data_result) {
			NANI_LOG_ERROR(m_logger, "Failed to load template data for path {}.", this_biome.path().string());
			continue;
		}
		auto template_data = std::move(*template_data_result);
		auto region = this_biome.path().filename().string();
		region = region.substr(0, region.find('.'));
		auto biome = template_data["biome"].is_string() ? template_data["biome"].as_string().data() : region;
		map_templates.push_back(MapTemplate{template_data, region, biome});
		NANI_LOG_INFO(m_logger, "Created template with region [{}] and biome [{}].", region, biome);
	}

	// populate map table
	for (auto const& this_region : std::filesystem::recursive_directory_iterator(room_list)) {
		if (!this_region.is_directory()) { continue; }
		NANI_LOG_INFO(m_logger, "Reading levels from folder: {}", this_region.path().filename().string());
		for (auto const& this_room : std::filesystem::recursive_directory_iterator(this_region)) {
			if (this_room.path().extension() != ".json") {
				NANI_LOG_ERROR(m_logger, "Found a file with an extension other than \".json\": {}.", this_room.path().filename().string());
				continue;
			}

			auto room_data_result = dj::Json::from_file(this_room.path().string());
			if (!room_data_result) {
				NANI_LOG_ERROR(m_logger, "Failed to load room data for path {}.", this_room.path().string());
				continue;
			}
			auto room_data = std::move(*room_data_result);

			auto this_id = room_data["meta"]["room_id"].as<int>();
			auto this_name = this_room.path().filename().string();
			auto this_biome = room_data["meta"]["biome"].is_string() ? room_data["meta"]["biome"].as_string().data() : this_region.path().filename().string();
			if (is_duplicate_room(this_id)) {
				NANI_LOG_ERROR(m_logger, "Found a file with a duplicate room ID: {}. ID: {}", this_room.path().filename().string(), this_id);
				continue;
			}
			auto room_str = this_room.path().filename().string();
			room_str = room_str.substr(0, room_str.find('.'));
			map_jsons.push_back(MapData{this_id, room_data, this_region.path().filename().string(), this_biome, room_str});

			// cache map layers
			sf::Vector2<std::uint32_t> dimensions{};
			dimensions.x = map_jsons.back().metadata["meta"]["dimensions"][0].as<int>();
			dimensions.y = map_jsons.back().metadata["meta"]["dimensions"][1].as<int>();
			map_layers.push_back(std::vector<std::unique_ptr<world::Layer>>{});
			auto& in_tile = map_jsons.back().metadata["tile"];
			auto ho = static_cast<bool>(in_tile["flags"]["obscuring"].as_bool());
			auto hro = static_cast<bool>(in_tile["flags"]["reverse_obscuring"].as_bool());
			std::uint8_t ctr{0u};
			for (auto& layer : in_tile["layers"].as_array()) {
				auto parallax = in_tile["parallax"][ctr].as<float>();
				auto ignore_lighting = in_tile["ignore_lighting"][ctr].as_bool();
				if (parallax == 0.f) { parallax = 1.f; }
				auto partition = sf::Vector2i{in_tile["middleground"].as<int>(), static_cast<int>(in_tile["layers"].as_array().size())};
				map_layers.back().push_back(std::make_unique<world::Layer>(ctr, partition, dimensions, in_tile["layers"][ctr], constants::f_cell_size, ho, hro, parallax, ignore_lighting));
				++ctr;
			}
			if (room_data["meta"]["minimap"].as_bool()) { minimap.bake(*m_services, room_data); }

			// write to map table
			auto entry = dj::Json{};
			entry["room_id"] = this_id;
			entry["label"] = this_name;
			entry["region"] = this_biome;
			entry["folder"] = this_region.path().filename().string();
			map_table["rooms"].push_back(entry);
			NANI_LOG_INFO(m_logger, "Added to map table: {}", this_name);
			NANI_LOG_INFO(m_logger, "In folder: {}", this_region.path().filename().string());
		}
	}
	if (!map_table.to_file((finder.resource_path() + "/data/level/map_table.json").c_str())) { NANI_LOG_ERROR(m_logger, "Failed to save map table!"); }
	map_table = *dj::Json::from_file((finder.resource_path() + "/data/level/map_table.json").c_str());
	assert(!map_table.is_null());
	for (auto const& room : map_table["rooms"].as_array()) {
		auto id = room["room_id"].as<int>();
		if (m_map_labels.contains(id)) { continue; }
		m_map_labels.insert(std::make_pair(id, room["label"].as_string()));
		rooms.push_back(room["room_id"].as<int>());
	}

	for (auto& id : discovered_rooms) {}

	auto ctr{0};
	for (auto& file : files) {
		file.id = ctr;
		file.label = "file_" + std::to_string(ctr);
		file.save_data = *dj::Json::from_file((finder.resource_path() + "/data/save/file_" + std::to_string(ctr) + ".json").c_str());
		if (file.save_data["status"]["new"].as_bool()) { file.flags.set(fornani::io::FileFlags::new_file); }
		++ctr;
	}
	blank_file.save_data = *dj::Json::from_file((finder.resource_path() + "/data/save/new_game.json").c_str());
	trial_file.save_data = *dj::Json::from_file((finder.resource_path() + "/data/save/trial_save.json").c_str());

	time_trial_data = *dj::Json::from_file((finder.resource_path() + "/data/save/time_trials.json").c_str());
	assert(!time_trial_data.is_null());
	for (auto const& course : time_trial_data["trials"].as_array()) {
		for (auto const& time : course["times"].as_array()) { time_trial_registry.insert_time(*m_services, course["course_id"].as<int>(), time["player_tag"].as_string().data(), time["time"].as<float>()); }
	}

	weapon = *dj::Json::from_file((finder.resource_path() + "/data/weapon/weapon_data.json").c_str());
	assert(!weapon.is_null());
	enemy_weapon = *dj::Json::from_file((finder.resource_path() + "/data/weapon/enemy_weapons.json").c_str());
	assert(!enemy_weapon.is_null());
	drop = *dj::Json::from_file((finder.resource_path() + "/data/item/drop.json").c_str());
	assert(!drop.is_null());
	particle = *dj::Json::from_file((finder.resource_path() + "/data/vfx/particle.json").c_str());
	assert(!particle.is_null());
	effect = *dj::Json::from_file((finder.resource_path() + "/data/vfx/effect.json").c_str());
	assert(!effect.is_null());
	sparkler = *dj::Json::from_file((finder.resource_path() + "/data/vfx/sparkler.json").c_str());
	assert(!sparkler.is_null());
	npc = *dj::Json::from_file((finder.resource_path() + "/data/npc/npc_data.json").c_str());
	assert(!npc.is_null());
	item = *dj::Json::from_file((finder.resource_path() + "/data/item/item.json").c_str());
	assert(!item.is_null());
	platform = *dj::Json::from_file((finder.resource_path() + "/data/level/platform.json").c_str());
	assert(!platform.is_null());
	cutscene = *dj::Json::from_file((finder.resource_path() + "/data/story/cutscenes.json").c_str());
	assert(!cutscene.is_null());
	map_styles = *dj::Json::from_file((finder.resource_path() + "/data/level/map_styles.json").c_str());
	assert(!map_styles.is_null());
	action_names = *dj::Json::from_file((finder.resource_path() + "/data/gui/action_names.json").c_str());
	assert(!action_names.is_null());
	light = *dj::Json::from_file((finder.resource_path() + "/data/vfx/light.json").c_str());
	assert(!light.is_null());

	enemy = *dj::Json::from_file((finder.resource_path() + "/data/enemy/enemy_params.json").c_str());
	assert(!enemy.is_null());

	frdog = *dj::Json::from_file((finder.resource_path() + "/data/enemy/frdog.json").c_str());
	assert(!frdog.is_null());
	hulmet = *dj::Json::from_file((finder.resource_path() + "/data/enemy/hulmet.json").c_str());
	assert(!hulmet.is_null());

	menu = *dj::Json::from_file((finder.resource_path() + "/data/gui/menu.json").c_str());
	assert(!menu.is_null());
	background = *dj::Json::from_file((finder.resource_path() + "/data/level/background_behaviors.json").c_str());
	assert(!background.is_null());

	// load marketplace
	for (auto const& entry : npc.as_object()) {
		if (!entry.second["vendor"]) { continue; }
		marketplace.insert({entry.second["vendor"]["id"].as<int>(), npc::Vendor()});
		auto& vendor = marketplace.at(entry.second["vendor"]["id"].as<int>());
		vendor.set_upcharge(entry.second["vendor"]["upcharge"].as<float>());
		for (auto& item : entry.second["vendor"]["common_items"].as_array()) { vendor.common_items.push_back(item.as_string().data()); }
		for (auto& item : entry.second["vendor"]["uncommon_items"].as_array()) { vendor.uncommon_items.push_back(item.as_string().data()); }
		for (auto& item : entry.second["vendor"]["rare_items"].as_array()) { vendor.rare_items.push_back(item.as_string().data()); }
	}

	// load item labels
	for (auto const& [key, entry] : item.as_object()) {
		if (m_item_labels.contains(entry["id"].as<int>())) { continue; }
		m_item_labels.insert({entry["id"].as<int>(), key});
	}

	gui_text = *dj::Json::from_file((finder.resource_path() + "/text/console/gui.json").c_str());
	assert(!gui_text.is_null());

	m_services->stopwatch.stop();
	m_services->stopwatch.print_time("data loaded");
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

	// write marketplace status
	save["marketplace"] = dj::Json::empty_array();
	for (auto& vendor : marketplace) {
		auto out_vendor = dj::Json::empty_array();
		// TODO: redo this once I add collectible items
		// for (auto& item : vendor.second.inventory.key_items_view()) { out_vendor.push_back(item.get_id()); }
		save["marketplace"].push_back(out_vendor);
	}

	m_services->quest_table.serialize(save);

	// write opened chests and doors
	save["map_data"]["world_time"]["hours"] = m_services->world_clock.get_hours();
	save["map_data"]["world_time"]["minutes"] = m_services->world_clock.get_minutes();
	save["piggybacker"] = player.get_piggybacker_id();
	save["npc_locations"] = dj::Json::empty_array();
	save["map_data"]["fallen_enemies"] = dj::Json::empty_array();
	save["discovered_rooms"] = dj::Json::empty_array();
	save["unlocked_doors"] = dj::Json::empty_array();
	save["opened_chests"] = dj::Json::empty_array();
	save["activated_switches"] = dj::Json::empty_array();
	save["destroyed_blocks"] = dj::Json::empty_array();
	save["destroyed_inspectables"] = dj::Json::empty_array();
	save["quest_progressions"] = dj::Json::empty_array();
	for (auto& location : npc_locations) {
		auto entry = dj::Json::empty_array();
		entry.push_back(location.first);
		entry.push_back(location.second);
		save["npc_locations"].push_back(entry);
	}
	for (auto& enemy : fallen_enemies) {
		auto entry = dj::Json::empty_array();
		entry.push_back(enemy.code.first);
		entry.push_back(enemy.code.second);
		entry.push_back(enemy.respawn_distance);
		entry.push_back(static_cast<int>(enemy.permanent));
		entry.push_back(static_cast<int>(enemy.semipermanent));
		save["map_data"]["fallen_enemies"].push_back(entry);
	}
	for (auto& room : discovered_rooms) { save["discovered_rooms"].push_back(room); }
	for (auto& door : unlocked_doors) { save["unlocked_doors"].push_back(door); }
	for (auto& chest : opened_chests) { save["opened_chests"].push_back(chest); }
	for (auto& s : activated_switches) { save["activated_switches"].push_back(s); }
	for (auto& block : destructible_states) {
		auto state = dj::Json{};
		state.push_back({block.first});
		state.push_back({block.second});
		save["destroyed_blocks"].push_back(state);
	}
	for (auto& i : destroyed_inspectables) { save["destroyed_inspectables"].push_back(i); }
	for (auto& q : quest_progressions) {
		auto out_quest = dj::Json::empty_array();
		out_quest.push_back(q.type);
		out_quest.push_back(q.id);
		out_quest.push_back(q.source_id);
		out_quest.push_back(q.amount);
		out_quest.push_back(q.hard_set);
		save["quest_progressions"].push_back(out_quest);
	}

	// save arsenal
	save["player_data"]["arsenal"] = dj::Json::empty_array();
	save["player_data"]["hotbar"] = dj::Json::empty_array();
	// push player arsenal
	if (player.arsenal) {
		for (auto& gun : player.arsenal.value().get_loadout()) { save["player_data"]["arsenal"].push_back(gun->get_tag()); }
		if (player.hotbar) {
			for (auto& id : player.hotbar.value().get_tags()) { save["player_data"]["hotbar"].push_back(id); }
			save["player_data"]["equipped_gun"] = player.hotbar.value().get_tag();
		}
	}

	// wardrobe
	save["player_data"]["wardrobe"]["hairstyle"] = static_cast<int>(player.catalog.wardrobe.get_variant(player::ApparelType::hairstyle));
	save["player_data"]["wardrobe"]["shirt"] = static_cast<int>(player.catalog.wardrobe.get_variant(player::ApparelType::shirt));
	save["player_data"]["wardrobe"]["pants"] = static_cast<int>(player.catalog.wardrobe.get_variant(player::ApparelType::pants));

	// items and abilities
	save["player_data"]["abilities"] = dj::Json::empty_array();
	save["player_data"]["items"] = dj::Json::empty_array();
	if (player.catalog.abilities.has_ability(player::AbilityType::dash)) { save["player_data"]["abilities"].push_back(1); }
	if (player.catalog.abilities.has_ability(player::AbilityType::wallslide)) { save["player_data"]["abilities"].push_back(0); }
	if (player.catalog.abilities.has_ability(player::AbilityType::doublejump)) { save["player_data"]["abilities"].push_back(2); }
	for (auto& item : player.catalog.inventory.items_view()) {
		dj::Json this_item{};
		this_item["label"] = item->get_label();
		this_item["quantity"] = 1;
		this_item["revealed"] = item->is_revealed();
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

	if (!save.dj::Json::to_file((m_services->finder.resource_path() + "/data/save/file_" + std::to_string(current_save) + ".json").c_str())) { NANI_LOG_ERROR(m_logger, "Failed to save file!"); }
}

void DataManager::save_settings() {
	settings["auto_sprint"] = m_services->controller_map.is_autosprint_enabled();
	settings["tutorial"] = m_services->tutorial();
	settings["gamepad"] = m_services->controller_map.is_gamepad_input_enabled();
	settings["music_volume"] = m_services->music_player.get_volume();
	settings["sfx_volume"] = m_services->soundboard.get_volume();
	settings["fullscreen"] = m_services->fullscreen();
	if (!settings.dj::Json::to_file((m_services->finder.resource_path() + "/data/config/settings.json").c_str())) { NANI_LOG_ERROR(m_logger, "Failed to save user settings!"); }
}

int DataManager::load_progress(player::Player& player, int const file, bool state_switch, bool from_menu) {

	current_save = file;
	auto const& save = files.at(file).save_data;
	assert(!save.is_null());

	// marketplace
	for (auto& vendor : marketplace) {
		vendor.second.inventory = {};
		for (auto& v : save["marketplace"].as_array()) {
			for (auto& id : v.as_array()) { vendor.second.inventory.add_item(item, item["label"].as_string()); }
		}
	}

	m_services->quest = {};
	discovered_rooms.clear();
	unlocked_doors.clear();
	opened_chests.clear();
	destructible_states.clear();
	activated_switches.clear();
	destroyed_inspectables.clear();
	quest_progressions.clear();
	npc_locations.clear();
	fallen_enemies.clear();

	m_services->quest_table.unserialize(save);

	m_services->world_clock.set_time(save["map_data"]["world_time"]["hours"].as<int>(), save["map_data"]["world_time"]["minutes"].as<int>());
	for (auto& room : save["discovered_rooms"].as_array()) { discovered_rooms.push_back(room.as<int>()); }
	for (auto& door : save["unlocked_doors"].as_array()) { unlocked_doors.push_back(door.as<int>()); }
	for (auto& chest : save["opened_chests"].as_array()) { opened_chests.push_back(chest.as<int>()); }
	for (auto& s : save["activated_switches"].as_array()) { activated_switches.push_back(s.as<int>()); }
	for (auto& block : save["destroyed_blocks"].as_array()) { destructible_states.push_back(std::make_pair(block[0].as<int>(), block[1].as<int>())); }
	for (auto& inspectable : save["destroyed_inspectables"].as_array()) { destroyed_inspectables.push_back(inspectable.as<int>()); }
	for (auto& q : save["quest_progressions"].as_array()) {
		auto type = q[0].as<int>();
		auto id = q[1].as<int>();
		auto srcid = q[2].as<int>();
		auto amt = q[3].as<int>();
		auto hard = q[4].as<int>();
		quest_progressions.push_back(util::QuestKey{type, id, srcid, amt, hard});
		m_services->quest.process(*m_services, quest_progressions.back());
	}
	for (auto& location : save["npc_locations"].as_array()) { npc_locations.insert({location[0].as<int>(), location[1].as<int>()}); }
	for (auto& enemy : save["map_data"]["fallen_enemies"].as_array()) {
		fallen_enemies.push_back({std::make_pair(enemy[0].as<int>(), enemy[1].as<int>()), enemy[2].as<int>(), static_cast<bool>(enemy[3].as<int>()), static_cast<bool>(enemy[4].as<int>())});
	};
	player.piggybacker = {};
	if (save["piggybacker"].as<int>() != 0) { player.piggyback(save["piggybacker"].as<int>()); }

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
	for (auto& gun_tag : save["player_data"]["arsenal"].as_array()) { player.push_to_loadout(gun_tag.as_string(), true); }
	if (!save["player_data"]["hotbar"].as_array().empty()) {
		if (!player.hotbar) { player.hotbar = arms::Hotbar(1); }
		if (player.hotbar) {
			for (auto& gun_tag : save["player_data"]["hotbar"].as_array()) { player.hotbar.value().add(gun_tag.as_string()); }
			auto equipped_gun = save["player_data"]["equipped_gun"].as_string();
			player.hotbar.value().set_selection(equipped_gun);
		}
	}

	// load items and abilities
	player.catalog.abilities.clear();
	player.catalog.inventory = {};
	for (auto& ability : save["player_data"]["abilities"].as_array()) { player.catalog.abilities.give_ability(ability.as<int>()); }
	for (auto& item : save["player_data"]["items"].as_array()) {
		player.give_item(item["label"].as_string(), item["quantity"].as<int>(), true);
		if (item["revealed"].as_bool()) { player.catalog.inventory.reveal_item(item_id_from_label(item["label"].as_string())); }
	}

	// wardrobe
	auto& wardrobe = player.catalog.wardrobe;
	auto hairstyle = save["player_data"]["wardrobe"]["hairstyle"].as<int>();
	auto headgear = save["player_data"]["wardrobe"]["headgear"].as<int>();
	auto shirt = save["player_data"]["wardrobe"]["shirt"].as<int>();
	auto pants = save["player_data"]["wardrobe"]["pants"].as<int>();
	player.set_outfit({hairstyle, headgear, shirt, pants});
	player.catalog.wardrobe.update(player.texture_updater);

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
	if (files.at(file).flags.test(fornani::io::FileFlags::new_file)) { s.player.death_count.set(0); }

	return room_id;
}

void DataManager::load_settings() {
	settings = *dj::Json::from_file((m_services->finder.resource_path() + "/data/config/settings.json").c_str());
	assert(!settings.is_null());
	m_services->controller_map.enable_autosprint(settings["auto_sprint"].as_bool());
	m_services->set_tutorial(settings["tutorial"].as_bool());
	m_services->controller_map.enable_gamepad_input(settings["gamepad"].as_bool());
	m_services->music_player.set_volume(settings["music_volume"].as<float>());
	m_services->soundboard.set_volume(settings["sfx_volume"].as<float>());
	m_services->set_fullscreen(settings["fullscreen"].as_bool());
	NANI_LOG_INFO(m_logger, "Enabled user settings.");
}

void DataManager::delete_file(int index) {
	if (index >= files.size()) { return; }
	files.at(index).save_data = blank_file.save_data;
	files.at(index).flags.set(fornani::io::FileFlags::new_file);
	if (!files.at(index).save_data.to_file((m_services->finder.resource_path() + "/data/save/file_" + std::to_string(current_save) + ".json").c_str())) { NANI_LOG_ERROR(m_logger, "Failed to clear save data!"); }
}

void DataManager::write_death_count(player::Player& player) {
	auto& save = files.at(current_save).save_data;
	auto& out_stat = save["player_data"]["stats"];
	auto const& s = m_services->stats;
	out_stat["death_count"] = s.player.death_count.get_count();
	if (!save.to_file((m_services->finder.resource_path() + "/data/save/file_" + std::to_string(current_save) + ".json").c_str())) { NANI_LOG_ERROR(m_logger, "Failed to write death count to save!"); }
}

std::string_view DataManager::load_blank_save(player::Player& player, bool state_switch) const {

	auto const& save = blank_file.save_data;
	assert(!save.is_null());

	// set player data based on save file
	player.health.set_max(save["player_data"]["max_hp"].as<float>());
	player.health.set_hp(save["player_data"]["hp"].as<float>());
	player.wallet.set_balance(save["player_data"]["orbs"].as<int>());

	// load player's arsenal
	player.arsenal = {};

	return m_map_labels.at(1);
}

void DataManager::load_trial_save(player::Player& player) const {

	auto const& save = trial_file.save_data;
	assert(!save.is_null());

	// set player data based on save file
	player.health.set_max(save["player_data"]["max_hp"].as<float>());
	player.health.set_hp(save["player_data"]["hp"].as<float>());
	for (auto& item : save["player_data"]["items"].as_array()) {
		player.give_item(item["label"].as_string(), item["quantity"].as<int>());
		if (item["revealed"].as_bool()) { player.catalog.inventory.reveal_item(item_id_from_label(item["label"].as_string())); }
	}

	// load player's arsenal
	player.arsenal = {};
}

void DataManager::load_player_params(player::Player& player) {

	// std::cout << "loading player params ...";
	player_params = *dj::Json::from_file((m_services->finder.resource_path() + "/data/player/physics_params.json").c_str());
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
	player.physics_stats.roll_speed = player_params["physics"]["roll_speed"].as<float>();
	player.physics_stats.slide_speed = player_params["physics"]["slide_speed"].as<float>();
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
	player_params["physics"]["roll_speed"] = player.physics_stats.roll_speed;
	player_params["physics"]["slide_speed"] = player.physics_stats.slide_speed;

	if (!player_params.dj::Json::to_file((finder.resource_path() + "/data/player/physics_params.json").c_str())) { NANI_LOG_ERROR(m_logger, "Failed to save physics params!"); }
}

void DataManager::open_chest(int id) { opened_chests.push_back(id); }

void DataManager::unlock_door(int id) { unlocked_doors.push_back(id); }

void DataManager::activate_switch(int id) {
	if (!switch_is_activated(id)) { activated_switches.push_back(id); }
}

void DataManager::switch_destructible_state(int id, bool inverse) {
	for (auto [i, d] : std::views::enumerate(destructible_states)) {
		if (d.first == id) {
			d.second = (d.second + 1) % 2;
			NANI_LOG_DEBUG(m_logger, "State was set to {} in DataManager.", d.second);
			return;
		}
	}
	auto state = inverse ? 0 : 1;
	destructible_states.push_back({id, state});
}

void DataManager::destroy_inspectable(int id) { destroyed_inspectables.push_back(id); }

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

void DataManager::kill_enemy(int room_id, int id, int distance, bool permanent, bool semipermanent) {
	for (auto& e : fallen_enemies) {
		if (e.code.first == room_id && e.code.second == id) { return; }
	}
	fallen_enemies.push_back({{room_id, id}, distance, permanent, semipermanent});
}

void DataManager::respawn_enemy(int room_id, int id) {
	std::erase_if(fallen_enemies, [room_id, id](auto const& i) { return i.code.first == room_id && i.code.second == id && !i.permanent; });
}

void DataManager::respawn_enemies(int room_id, int distance) {
	std::erase_if(fallen_enemies, [room_id, distance](auto const& i) { return i.code.first == room_id && i.respawn_distance < distance && !i.permanent; });
}

void DataManager::respawn_all() {
	std::erase_if(fallen_enemies, [](auto const& i) { return !i.permanent && !i.semipermanent; });
}

bool data::DataManager::is_duplicate_room(int id) const {
	for (auto& json : map_jsons) {
		if (json.id == id) {
			NANI_LOG_ERROR(m_logger, ">>> found a dup! room: {}, ID: {}", json.room_label, id);
			return true;
		}
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

bool DataManager::inspectable_is_destroyed(int id) const {
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

int DataManager::get_destructible_state(int id) const {
	for (auto [i, d] : std::views::enumerate(destructible_states)) {
		if (d.first == id) { return d.second; }
	}
	return -1;
}

void DataManager::load_controls(config::ControllerMap& controller) {
	// XXX change controls json when keybinds get modified
	controls = *dj::Json::from_file((m_services->finder.resource_path() + "/data/config/control_map.json").c_str());
	assert(!controls.is_null());
	assert(controls["controls"] && controls["controls"].is_object());

	for (auto const& [key, item] : controls["controls"].as_object()) {
		assert(item.is_object());
		if (item.as_object().contains("primary_key")) { controller.set_primary_keyboard_binding(controller.get_action_by_identifier(key), controller.string_to_key(item["primary_key"].as_string())); }
	}
}

void DataManager::save_controls(config::ControllerMap& controller) {
	if (!controls.dj::Json::to_file((m_services->finder.resource_path() + "/data/config/control_map.json").c_str())) { NANI_LOG_ERROR(m_logger, "Failed to save controls layout!"); }
}

void DataManager::reset_controls() { controls = *dj::Json::from_file((m_services->finder.resource_path() + "/data/config/defaults.json").c_str()); }

auto DataManager::item_id_from_label(std::string_view label) const -> int {
	for (auto const& l : m_item_labels) {
		if (l.second == label) { return l.first; }
	}
	return 0;
}

auto DataManager::get_gun_tag_from_id(int id) const -> std::optional<std::string_view> {
	for (auto const& gun : weapon.as_object()) {
		if (gun.second["metadata"]["id"].as<int>() == id) { return gun.first; }
	}
	return std::nullopt;
}

auto DataManager::get_gun_id_from_tag(std::string_view tag) const -> int { return weapon[tag]["metadata"]["id"].as<int>(); }

auto DataManager::get_room_data_from_id(int id) const& -> std::optional<dj::Json> {
	for (auto const& room : map_table["rooms"].as_array()) {
		if (room["room_id"].as<int>() == id) {
			return room;
			NANI_LOG_DEBUG(m_logger, "Found room {}", id);
		}
	}
	return std::nullopt;
}

auto DataManager::get_npc_label_from_id(int id) const -> std::optional<std::string_view> {
	for (auto const& [key, entry] : npc.as_object()) {
		if (entry["id"].as<int>() == id) { return key; }
	}
	return std::nullopt;
}

auto DataManager::get_enemy_label_from_id(int id) const -> std::optional<std::string_view> {
	for (auto const& [key, entry] : enemy.as_object()) {
		if (entry["metadata"]["id"].as<int>() == id) { return key; }
	}
	return std::nullopt;
}

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

std::vector<std::unique_ptr<world::Layer>>& DataManager::get_layers(int id) { return map_layers.at(get_room_index(id)); }

} // namespace fornani::data
