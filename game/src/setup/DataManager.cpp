
#include "fornani/setup/DataManager.hpp"
#include <fornani/core/Common.hpp>
#include <fornani/graphics/MenuTheme.hpp>
#include <fornani/io/Codec.hpp>
#include <fornani/io/FileSerializer.hpp>
#include <fstream>
#include "fornani/entities/player/Player.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/systems/InputSystem.hpp"

namespace fornani::data {

DataManager::DataManager(automa::ServiceProvider& svc) : m_services(&svc), minimap{svc} {
	load_data();
	// load themes
	auto themes_result = dj::Json::from_file((svc.finder.resource_path() + "/data/gui/menu_themes.json").c_str());
	if (!themes_result) {
		NANI_LOG_ERROR(m_logger, "Failed to load themes!");
		return;
	}
	menu_themes = std::move(*themes_result);
	assert(!menu_themes.is_null());
}

void DataManager::load_data() {
	m_services->stopwatch.start();
	NANI_LOG_INFO(m_logger, "Data loading started.");
	auto const& finder = m_services->finder;

	// save files
	for (auto [i, file] : std::views::enumerate(files)) {
		file.id = i;
		file.label = "file_" + std::to_string(i);
		auto filename = finder.paths.save / fs::path{"file_" + std::to_string(i) + ".sav"};
		auto template_file = finder.resource_path() / fs::path{"data/save/new_game.json"};
		finder.ensure_save_exists(filename, template_file);
	}

	auto time_trials_file = (finder.paths.save / fs::path{"time_trials.sav"});
	auto time_trials_template = finder.resource_path() / fs::path{"data/config/time_trials.json"};
	finder.ensure_save_exists(time_trials_file, time_trials_template);
	load_time_trials_binary(time_trials_file);

	blank_file.save_data = *dj::Json::from_file((finder.resource_path() + "/data/save/new_game.json").c_str());
	trial_file.save_data = *dj::Json::from_file((finder.resource_path() + "/data/save/trial_save.json").c_str());

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
			if (is_duplicate_room(this_id)) { continue; }
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
		}
	}
	NANI_LOG_INFO(m_logger, "Number of maps loaded: {}", map_jsons.size());
	NANI_LOG_INFO(m_logger, "Number of maps added to table: {}", map_table["rooms"].as_array().size());
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
	fader = *dj::Json::from_file((finder.resource_path() + "/data/vfx/fader.json").c_str());
	assert(!fader.is_null());
	sparkler = *dj::Json::from_file((finder.resource_path() + "/data/vfx/sparkler.json").c_str());
	assert(!sparkler.is_null());
	npc = *dj::Json::from_file((finder.resource_path() + "/data/npc/npc_data.json").c_str());
	assert(!npc.is_null());
	item = *dj::Json::from_file((finder.resource_path() + "/data/item/item.json").c_str());
	assert(!item.is_null());
	props = *dj::Json::from_file((finder.resource_path() + "/data/level/props.json").c_str());
	assert(!props.is_null());

	platform = *dj::Json::from_file((finder.resource_path() + "/data/level/platform.json").c_str());
	assert(!platform.is_null());
	cutscene = *dj::Json::from_file((finder.resource_path() + "/data/story/cutscenes.json").c_str());
	assert(!cutscene.is_null());
	biomes = *dj::Json::from_file((finder.resource_path() + "/data/level/biomes.json").c_str());
	assert(!biomes.is_null());
	for (auto const& b : biomes["biomes"].as_array()) { m_biomes.push_back(b.as_string()); }
	hazards = *dj::Json::from_file((finder.resource_path() + "/data/level/tables/hazards.json").c_str());
	assert(!hazards.is_null());

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
		if (!entry.second["vendor"].is_object()) { continue; }
		marketplace.insert({entry.second["id"].as<int>(), npc::Vendor()});
		auto& vendor = marketplace.at(entry.second["id"].as<int>());
		vendor.set_upcharge(entry.second["vendor"]["upcharge"].as<float>());
		for (auto& item : entry.second["vendor"]["common_items"].as_array()) { vendor.common_items.push_back(item.as_string().data()); }
		for (auto& item : entry.second["vendor"]["uncommon_items"].as_array()) { vendor.uncommon_items.push_back(item.as_string().data()); }
		for (auto& item : entry.second["vendor"]["rare_items"].as_array()) { vendor.rare_items.push_back(item.as_string().data()); }
		for (auto& item : entry.second["vendor"]["guaranteed_finite_items"].as_array()) { vendor.guaranteed_finite_items.push_back(item.as_string().data()); }
		NANI_LOG_INFO(m_logger, "Created Vendor in marketplace with ID {}", entry.second["id"].as<int>());
	}

	for (auto [i, item] : std::views::enumerate(item.as_array())) { NANI_LOG_INFO(m_logger, "{} : {}", item["tag"].as_string(), i); }

	m_services->stopwatch.stop();
	// m_services->stopwatch.print_time("data loaded");
	m_services->stopwatch.start();
}

void DataManager::save_quests() {
	auto& save = files.at(current_save).save_data;
	files.at(current_save).write();
	m_services->quest_table.serialize(save);
}

void DataManager::save_progress(player::Player& player, int save_point_id) {
	auto& save = files.at(current_save).save_data;
	files.at(current_save).write();

	// set file data based on player state
	save["save_point_id"] = save_point_id;

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
	m_services->world_clock.serialize(save["map_data"]["world_time"]);
	save["piggybacker"] = player.get_piggybacker_id();
	save["map_data"]["fallen_enemies"] = dj::Json::empty_array();
	save["discovered_rooms"] = dj::Json::empty_array();
	save["unlocked_doors"] = dj::Json::empty_array();
	save["opened_chests"] = dj::Json::empty_array();
	save["activated_switches"] = dj::Json::empty_array();
	save["destroyed_blocks"] = dj::Json::empty_array();
	save["destroyed_inspectables"] = dj::Json::empty_array();
	for (auto& enemy : fallen_enemies) {
		auto entry = dj::Json::empty_array();
		entry.push_back(enemy.code.first);
		entry.push_back(enemy.code.second.get());
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

	player.serialize(save["player_data"]);

	// stat tracker
	auto& out_stat = save["player_data"]["stats"];
	auto const& s = m_services->stats;
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

	save_current();
}

void DataManager::save_current() {
	auto& save = files.at(current_save).save_data;
	auto path = m_services->finder.paths.save / ("file_" + std::to_string(current_save) + ".sav");
	std::ofstream out(path, std::ios::binary);
	if (!out) {
		NANI_LOG_ERROR(m_logger, "Failed to open save file!");
		return;
	}
	auto json = save.serialize();
	if (!codec::encode(json, out)) { NANI_LOG_ERROR(m_logger, "Failed while writing save file!"); }
}

void DataManager::serialize_death() {
	auto& save = files.at(current_save).save_data;
	auto current_deaths = save["player_data"]["stats"]["death_count"].as<int>();
	save["player_data"]["stats"]["death_count"] = current_deaths + 1;
	save_current();
}

void DataManager::load_localized_data(AppContext& ctx) {
	auto gui_text_result = dj::Json::from_file((m_services->finder.resource_path() + ctx.localization.get_folder_string() + "/gui.json").c_str());
	if (!gui_text_result) {
		NANI_LOG_ERROR(m_logger, "Failed to load gui text!");
		return;
	}
	gui_text = std::move(*gui_text_result);
	assert(!gui_text.is_null());
}

int DataManager::reload_progress(player::Player& player) { return load_progress(player, current_save, true); }

int DataManager::load_progress(player::Player& player, int const file, bool state_switch) {
	current_save = file;
	auto base = "file_" + std::to_string(current_save);
	auto sav_path = m_services->finder.paths.save / (base + ".sav");
	auto json_path = m_services->finder.paths.save / (base + ".json");

	if (fs::exists(sav_path)) {
		NANI_LOG_INFO(m_logger, "Loading save from binary: {}", sav_path.string());
		if (!load_save_binary(sav_path, player)) {
			NANI_LOG_ERROR(m_logger, "Failed save from binary!!");
			return 0;
		}
		if (!load_save_json(json_path, player, state_switch)) {
			NANI_LOG_ERROR(m_logger, "Failed save from json!!");
			return 0;
		}
	} else if (fs::exists(json_path)) {
		NANI_LOG_INFO(m_logger, "Loading save from json: {}", json_path.string());
		if (!load_save_json(json_path, player)) { return 0; }
	}
	return files.at(current_save).save_data["save_point_id"].as<int>();
}

void DataManager::delete_file(int index) {
	if (index >= files.size()) { return; }
	auto filename = m_services->finder.paths.save / fs::path{"file_" + std::to_string(index) + ".sav"};
	auto template_file = m_services->finder.resource_path() / fs::path{"data/save/new_game.json"};
	m_services->finder.overwrite_save(filename, template_file);
}

std::string_view DataManager::load_blank_save(player::Player& player, bool state_switch) const {

	auto const& save = blank_file.save_data;
	assert(!save.is_null());

	// set player data based on save file
	player.health.set_capacity(save["player_data"]["max_hp"].as<float>());
	player.health.set_quantity(save["player_data"]["hp"].as<float>());
	player.wallet.set_balance(save["player_data"]["orbs"].as<int>());

	// load player's arsenal
	player.arsenal = {};

	return m_map_labels.at(1);
}

void DataManager::load_trial_save(player::Player& player) const {
	auto const& save = trial_file.save_data;
	assert(!save.is_null());
	player.unserialize(save["player_data"]);
}

void DataManager::load_player_params(player::Player& player) {
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

void DataManager::open_chest(std::uint64_t id) { opened_chests.add(id); }

void DataManager::reveal_room(int id) { discovered_rooms.add(id); }

void DataManager::unlock_door(std::string_view tag) { unlocked_doors.add(tag.data()); }

void DataManager::activate_switch(int id) {
	if (!activated_switches.contains(id)) {
		activated_switches.add(id);
		auto& save = files.at(current_save).save_data;
		save["activated_switches"] = dj::Json::empty_array();
		for (auto& s : activated_switches) { save["activated_switches"].push_back(s); }
		save_current();
	}
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

void DataManager::destroy_inspectable(int id) { destroyed_inspectables.add(id); }

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

void DataManager::kill_enemy(int room_id, StableID id, int distance, bool permanent, bool semipermanent) {
	for (auto& e : fallen_enemies) {
		if (e.code.first == room_id && e.code.second == id) { return; }
	}
	fallen_enemies.push_back({{room_id, id}, distance, permanent, semipermanent});
}

void DataManager::respawn_enemy(int room_id, StableID id) {
	std::erase_if(fallen_enemies, [room_id, id](auto const& i) { return i.code.first == room_id && i.code.second == id && !i.permanent; });
}

void DataManager::respawn_enemies(int room_id, int distance) {
	std::erase_if(fallen_enemies, [room_id, distance](auto const& i) { return i.code.first == room_id && i.respawn_distance < distance && !i.permanent; });
	NANI_LOG_DEBUG(m_logger, "Tried to respawn enemies for room: {} from distance {}", room_id, distance);
}

void DataManager::respawn_all() {
	std::erase_if(fallen_enemies, [](auto const& i) { return !i.permanent && !i.semipermanent; });
}

bool data::DataManager::is_duplicate_room(int id) const {
	for (auto& json : map_jsons) {
		if (json.id == id) { return true; }
	}
	return false;
}

bool DataManager::is_door_unlocked(std::string_view tag) const { return unlocked_doors.contains(tag.data()); }

bool DataManager::chest_is_open(std::uint64_t id) const { return opened_chests.contains(id); }

bool DataManager::switch_is_activated(int id) const {
	for (auto& s : activated_switches) {
		if (s == id) { return true; }
	}
	return false;
}

bool DataManager::inspectable_is_destroyed(int id) const { return destroyed_inspectables.contains(id); }

bool DataManager::is_room_discovered(int id) const { return discovered_rooms.contains(id); }

bool DataManager::is_room_adjacent_to_discovered(int id) const {
	// loop over adjacent rooms
	auto result = get_map_json_from_id(id);
	if (!result) { return false; }
	auto const& in = std::move(*result).get();
	for (auto const& portal : in["entities"]["portals"].as_array()) {

		if (is_room_discovered(portal["destination_id"].as<int>())) { return true; }
	}

	return false;
}

bool DataManager::enemy_is_fallen(int room_id, StableID id) const {
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

auto DataManager::item_id_from_label(std::string_view label) const -> int {
	auto const& arr = item.as_array();
	auto it = std::find_if(arr.begin(), arr.end(), [&](auto const& v) { return v["tag"].as_string_view() == label; });

	if (it == arr.end()) {
		// NANI_LOG_ERROR(m_logger, "Failed to find item with label {} in item.json!", label);
		// NANI_LOG_ERROR(m_logger, "Item json size: {}", arr.size());
		return -1;
	}
	return static_cast<int>(std::distance(arr.begin(), it));
}

auto DataManager::get_gun_tag_from_id(int id) const -> std::optional<std::string_view> {
	for (auto const& gun : weapon.as_object()) {
		if (gun.second["metadata"]["id"].as<int>() == id) { return gun.first; }
	}
	return std::nullopt;
}

auto DataManager::get_gun_id_from_tag(std::string_view tag) const -> int { return weapon[tag]["metadata"]["id"].as<int>(); }

auto DataManager::get_map_data_from_id(int id) const -> std::optional<std::reference_wrapper<MapData const>> {
	for (auto const& map : map_jsons) {
		if (map.metadata["meta"]["room_id"].as<int>() == id) { return map; }
	}
	return std::nullopt;
}

auto DataManager::get_map_json_from_id(int id) const -> std::optional<std::reference_wrapper<dj::Json const>> {
	for (auto const& map : map_jsons) {
		if (map.metadata["meta"]["room_id"].as<int>() == id) { return map.metadata; }
	}
	return std::nullopt;
}

auto DataManager::get_map_json_from_id(int id) -> std::optional<std::reference_wrapper<dj::Json>> {
	auto const_result = static_cast<DataManager const*>(this)->get_map_json_from_id(id);
	if (!const_result) return std::nullopt;
	return std::ref(const_cast<dj::Json&>(const_result->get()));
}

auto DataManager::get_item_json_from_tag(std::string_view tag) const -> dj::Json const& { return item[item_id_from_label(tag)]; }

auto DataManager::get_gun_json_from_tag(std::string_view tag) const -> dj::Json const& { return weapon[tag]; }

auto DataManager::get_room_data_from_id(int id) const -> std::optional<dj::Json> {
	for (auto const& room : map_table["rooms"].as_array()) {
		if (room["room_id"].as<int>() == id) { return room; }
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

auto DataManager::get_npc_location(automa::ServiceProvider& svc, std::string_view label) const -> int {
	auto const& npc_data = npc[label];
	if (npc_data["locations"]) {
		auto tag = npc_data["locations"]["contingency"]["tag"].as_string();
		auto status = svc.quest_table.get_quest_progression(tag);
		auto const& target_data = npc_data["locations"]["statuses"][status];
		auto interval = static_cast<WorldClockInterval>(target_data["interval"].as<int>());
		auto chance = svc.world_clock.get_rng(interval);
		auto cumulative = 0.f;
		for (auto const& room : target_data["distributions"].as_array()) {
			cumulative += room["weight"].as<float>();
			if (chance < cumulative) { return room["room"].as<int>(); }
		}
		if (target_data["schedule"]) {
			auto schedule = NPCSchedule(target_data["schedule"]);
			return schedule.get_location(svc.world_clock.get_time_of_day());
		}
	}
	return 0;
}

std::vector<std::unique_ptr<world::Layer>>& DataManager::get_layers(int id) { return map_layers.at(get_room_index(id)); }

bool DataManager::load_save_binary(fs::path const& path, player::Player& player) {
	std::ifstream in(path, std::ios::binary);
	if (!in) {
		NANI_LOG_ERROR(m_logger, "Failed to open save file!");
		return false;
	}

	std::string json;
	if (!codec::decode(in, json)) {
		NANI_LOG_ERROR(m_logger, "Failed to decode save file!");
		return false;
	}

	auto result = dj::Json::parse(json);
	if (!result) {
		NANI_LOG_ERROR(m_logger, "Failed to parse JSON!");
		return false;
	}

	auto& file = files.at(current_save);
	file.save_data = std::move(*result);

	return true;
}

bool DataManager::load_save_json(fs::path const& path, player::Player& player, bool reload) {

	auto blank_template = m_services->finder.resource_path() / fs::path{"data/save/new_game.json"};
	auto blank_result = dj::Json::from_file(blank_template.string());
	if (!blank_result) { NANI_LOG_ERROR(m_logger, "Failed to clear out save data!"); }
	auto& save = files.at(current_save).save_data;
	// if (reload) { save = std::move(*blank_result); }

	assert(!save.is_null());

	// marketplace
	for (auto& vendor : marketplace) {}

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

	m_services->world_clock.unserialize(save["map_data"]["world_time"]);

	for (auto& room : save["discovered_rooms"].as_array()) { discovered_rooms.add(room.as<int>()); }
	for (auto& door : save["unlocked_doors"].as_array()) { unlocked_doors.add(door.as_string()); }
	for (auto& chest : save["opened_chests"].as_array()) { opened_chests.add(chest.as<std::uint64_t>()); }
	for (auto& s : save["activated_switches"].as_array()) { activated_switches.add(s.as<int>()); }
	for (auto& block : save["destroyed_blocks"].as_array()) { destructible_states.push_back(std::make_pair(block[0].as<int>(), block[1].as<int>())); }
	for (auto& inspectable : save["destroyed_inspectables"].as_array()) { destroyed_inspectables.add(inspectable.as<int>()); }

	for (auto& enemy : save["map_data"]["fallen_enemies"].as_array()) {
		fallen_enemies.push_back({std::make_pair(enemy[0].as<int>(), StableID{enemy[1].as<StableID::underlying_type>()}), enemy[2].as<int>(), static_cast<bool>(enemy[3].as<int>()), static_cast<bool>(enemy[4].as<int>())});
	};
	player.piggybacker = {};
	if (save["piggybacker"].as<int>() != 0) { player.piggyback(save["piggybacker"].as<int>()); }

	int save_pt_id = save["save_point_id"].as<int>();
	int room_id = save_pt_id;
	m_services->state_controller.save_point_id = save_pt_id;

	// set player data based on save file
	// in the future, all player data will be unserialized from this function
	player.unserialize(save["player_data"]);

	// stat tracker
	auto& s = m_services->stats;
	auto deaths = s.player.death_count.get_count();
	s = {};
	auto const& in_stat = save["player_data"]["stats"];
	s.player.death_count.set(in_stat["death_count"].as<int>());
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
	if (files.at(current_save).is_new()) { s.player.death_count.set(0); }

	return true;
}

bool DataManager::load_time_trials_binary(fs::path const& path) {
	std::ifstream in(path, std::ios::binary);
	if (!in) {
		NANI_LOG_ERROR(m_logger, "Failed to open save file!");
		return false;
	}

	std::string json;
	if (!codec::decode(in, json)) {
		NANI_LOG_ERROR(m_logger, "Failed to decode save file!");
		return false;
	}

	auto result = dj::Json::parse(json);
	if (!result) {
		NANI_LOG_ERROR(m_logger, "Failed to parse JSON for time trials!");
		return false;
	}

	time_trial_data = std::move(*result);
	assert(!time_trial_data.is_null());
	for (auto const& course : time_trial_data["trials"].as_array()) {
		for (auto const& time : course["times"].as_array()) { time_trial_registry.insert_time(*m_services, course["course_id"].as<int>(), time["player_tag"].as_string().data(), time["time"].as<float>()); }
	}

	return true;
}

} // namespace fornani::data
