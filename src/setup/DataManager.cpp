
#include "DataManager.hpp"
#include "../entities/player/Player.hpp"
#include "../service/ServiceProvider.hpp"
#include "ControllerMap.hpp"

namespace data {

DataManager::DataManager(automa::ServiceProvider& svc) : m_services(&svc) {}

void DataManager::load_data(std::string in_room) {

	map_table = dj::Json::from_file((finder.resource_path + "/data/level/map_table.json").c_str());
	assert(!map_table.is_null());
	for (auto const& room : map_table["rooms"].array_view()) { m_services->tables.get_map_label.insert(std::make_pair(room["room_id"].as<int>(), room["label"].as_string())); }

	// load map
	// std::cout << "loading map data...";
	int room_counter{};
	for (auto& room : rooms) {
		map_jsons.push_back(MapData());
		map_jsons.back().id = room;
		std::string room_str = m_services->tables.get_map_label.contains(room) ? finder.resource_path + "/level/" + m_services->tables.get_map_label.at(room) : finder.resource_path + "/level/" + in_room;
		map_jsons.back().metadata = dj::Json::from_file((room_str + "/meta.json").c_str());
		assert(!map_jsons.back().metadata.is_null());
		map_jsons.back().tiles = dj::Json::from_file((room_str + "/tile.json").c_str());
		assert(!map_jsons.back().tiles.is_null());
		map_jsons.back().inspectable_data = dj::Json::from_file((room_str + "/inspectables.json").c_str());
		// cache map layers
		int layer_counter{};
		sf::Vector2<uint32_t> dimensions{};
		dimensions.x = map_jsons.back().metadata["meta"]["dimensions"][0].as<int>();
		dimensions.y = map_jsons.back().metadata["meta"]["dimensions"][1].as<int>();
		map_layers.push_back(std::vector<world::Layer>());
		for (int i = 0; i < num_layers; ++i) { map_layers.at(room_counter).push_back(world::Layer(i, (i == world::MIDDLEGROUND), dimensions)); }
		for (auto& layer : map_layers.at(room_counter)) {
			int cell_counter{};
			layer.grid = world::Grid(dimensions);
			for (auto& cell : map_jsons.back().tiles["layers"][layer_counter].array_view()) {
				layer.grid.cells.at(cell_counter).value = cell.as<int>();
				++cell_counter;
			}
			layer.grid.seed_vertices();
			++layer_counter;
		}
		++room_counter;
	}

	auto ctr{0};
	for (auto& file : files) {
		file.id = ctr;
		file.label = "file_" + std::to_string(ctr);
		file.save_data = dj::Json::from_file((finder.resource_path + "/data/save/file_" + std::to_string(ctr) + ".json").c_str());
		if ((dj::Boolean)file.save_data["status"]["new"].as_bool()) { file.flags.set(fornani::FileFlags::new_file); }
		++ctr;
	}
	blank_file.save_data = dj::Json::from_file((finder.resource_path + "/data/save/new_game.json").c_str());
	// std::cout << " success!\n";

	// std::cout << "loading json data...";
	game_info = dj::Json::from_file((finder.resource_path + "/data/config/version.json").c_str());
	assert(!game_info.is_null());
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
	platform = dj::Json::from_file((finder.resource_path + "/data/level/platform.json").c_str());
	assert(!platform.is_null());
	map_styles = dj::Json::from_file((finder.resource_path + "/data/level/map_styles.json").c_str());
	assert(!map_styles.is_null());

	enemy = dj::Json::from_file((finder.resource_path + "/data/enemy/enemy_params.json").c_str());
	assert(!enemy.is_null());
	frdog = dj::Json::from_file((finder.resource_path + "/data/enemy/frdog.json").c_str());
	assert(!frdog.is_null());
	hulmet = dj::Json::from_file((finder.resource_path + "/data/enemy/hulmet.json").c_str());
	assert(!hulmet.is_null());

	menu = dj::Json::from_file((finder.resource_path + "/data/gui/menu.json").c_str());
	assert(!menu.is_null());
	background = dj::Json::from_file((finder.resource_path + "/data/level/background_behaviors.json").c_str());
	assert(!background.is_null());
	// std::cout << " success!\n";
}

void DataManager::save_progress(player::Player& player, int save_point_id) {

	auto& save = files.at(current_save).save_data;
	files.at(current_save).write();
	// set file data based on player state
	save["player_data"]["max_hp"] = player.health.get_max();
	save["player_data"]["hp"] = player.health.get_hp();
	save["player_data"]["orbs"] = player.player_stats.orbs;
	save["player_data"]["position"]["x"] = player.collider.physics.position.x;
	save["player_data"]["position"]["y"] = player.collider.physics.position.y;

	// create empty json array
	constexpr auto empty_array = R"([])";
	auto const wipe = dj::Json::parse(empty_array);

	// write opened chests and doors
	save["discovered_rooms"] = wipe;
	save["unlocked_doors"] = wipe;
	save["opened_chests"] = wipe;
	save["activated_switches"] = wipe;
	save["destroyed_blocks"] = wipe;
	save["destroyed_inspectables"] = wipe;
	for (auto& entry : save["quest_progressions"].array_view()) { entry = wipe; }
	save["quest_progressions"] = wipe;
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
	// push player arsenal
	if (player.arsenal) {
		for (auto& gun : player.arsenal.value().get_loadout()) {
			int this_id = gun->get_id();
			save["player_data"]["arsenal"].push_back(this_id);
		}
		save["player_data"]["equipped_gun"] = player.arsenal.value().get_index();
	}

	// items and abilities
	save["player_data"]["abilities"] = wipe;
	save["player_data"]["items"] = wipe;
	if (player.catalog.categories.abilities.has_ability(player::Abilities::dash)) { save["player_data"]["abilities"].push_back("dash"); }
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

	save.dj::Json::to_file((finder.resource_path + "/data/save/file_" + std::to_string(current_save) + ".json").c_str());
}

int DataManager::load_progress(player::Player& player, int const file, bool state_switch, bool from_menu) {

	current_save = file;
	auto const& save = files.at(file).save_data;
	assert(!save.is_null());

	m_services->quest = {};
	discovered_rooms.clear();
	unlocked_doors.clear();
	opened_chests.clear();
	destroyed_blocks.clear();
	activated_switches.clear();
	destroyed_inspectables.clear();
	quest_progressions.clear();
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
		m_services->quest.process(quest_progressions.back());
	}

	player.tutorial.flags = {};
	if (save["tutorial"]["jump"].as_bool()) { player.tutorial.flags.set(text::TutorialFlags::jump); }
	if (save["tutorial"]["shoot"].as_bool()) { player.tutorial.flags.set(text::TutorialFlags::shoot); }
	if (save["tutorial"]["sprint"].as_bool()) { player.tutorial.flags.set(text::TutorialFlags::sprint); }
	if (save["tutorial"]["map"].as_bool()) { player.tutorial.flags.set(text::TutorialFlags::map); }
	if (save["tutorial"]["inventory"].as_bool()) { player.tutorial.flags.set(text::TutorialFlags::inventory); }
	player.tutorial.current_state = static_cast<text::TutorialFlags>(save["tutorial"]["state"].as<int>());
	if (save["tutorial"]["closed"].as_bool()) { player.tutorial.close_for_good(); }
	player.cooldowns.tutorial.start();
	if (save["tutorial"]["on"].as_bool()) {
		player.tutorial.turn_on();
		player.tutorial.trigger();
	}

	int save_pt_id = save["save_point_id"].as<int>();
	int room_id = save_pt_id;
	m_services->state_controller.save_point_id = save_pt_id;

	// set player data based on save file
	player.health.set_max(save["player_data"]["max_hp"].as<float>());
	player.health.set_hp(save["player_data"]["hp"].as<float>());
	player.player_stats.orbs = save["player_data"]["orbs"].as<int>();

	// load player's arsenal
	player.arsenal = {};
	if (!save["player_data"]["arsenal"].array_view().empty()) { player.arsenal = arms::Arsenal(*m_services); }
	for (auto& gun_id : save["player_data"]["arsenal"].array_view()) {
		if (player.arsenal) { player.arsenal.value().push_to_loadout(gun_id.as<int>()); }
	}
	if (player.arsenal) {
		auto equipped_gun = save["player_data"]["equipped_gun"].as<int>();
		player.arsenal.value().set_index(equipped_gun);
	}

	// load items and abilities
	player.catalog.categories.abilities.clear();
	player.catalog.categories.inventory.clear();
	for (auto& ability : save["player_data"]["abilities"].array_view()) { player.catalog.categories.abilities.give_ability(ability.as_string()); }
	for (auto& item : save["player_data"]["items"].array_view()) { player.catalog.categories.inventory.add_item(*m_services, item["id"].as<int>(), item["quantity"].as<int>()); }

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
	m_services->ticker.in_game_seconds_passed = m_services->stats.float_to_seconds(in_stat["seconds_played"].as<float>());

	return room_id;
}

void DataManager::write_death_count(player::Player& player) {
	auto& save = files.at(current_save).save_data;
	auto& out_stat = save["player_data"]["stats"];
	auto const& s = m_services->stats;
	out_stat["death_count"] = s.player.death_count.get_count();
	save.dj::Json::to_file((finder.resource_path + "/data/save/file_" + std::to_string(current_save) + ".json").c_str());
}

std::string_view DataManager::load_blank_save(player::Player& player, bool state_switch) {

	auto const& save = blank_file.save_data;
	assert(!save.is_null());

	// set player data based on save file
	player.health.set_max(save["player_data"]["max_hp"].as<float>());
	player.health.set_hp(save["player_data"]["hp"].as<float>());
	player.player_stats.orbs = save["player_data"]["orbs"].as<int>();

	// load player's arsenal
	player.arsenal = {};

	return m_services->tables.get_map_label.at(100);
}

void DataManager::load_player_params(player::Player& player) {

	// std::cout << "loading player params ...";
	player_params = dj::Json::from_file((finder.resource_path + "/data/player/physics_params.json").c_str());
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

	player_params.dj::Json::to_file((finder.resource_path + "/data/player/physics_params.json").c_str());
	// std::cout << " success!\n";
}

void DataManager::open_chest(int id) { opened_chests.push_back(id); }

void DataManager::unlock_door(int id) { unlocked_doors.push_back(id); }

void DataManager::activate_switch(int id) {
	if (!switch_is_activated(id)) { activated_switches.push_back(id); }
}

void DataManager::destroy_block(int id) { destroyed_blocks.push_back(id); }

void DataManager::destroy_inspectable(std::string_view id) { destroyed_inspectables.push_back(id.data()); }

void DataManager::push_quest(util::QuestKey key) {
	for (auto& entry : quest_progressions) {
		if (entry == key) { return; }
	}
	quest_progressions.push_back(key);
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

void DataManager::load_controls(config::ControllerMap& controller) {

	controls = dj::Json::from_file((finder.resource_path + "/data/config/control_map.json").c_str());
	assert(!controls.is_null());

	controller.key_to_label.clear();
	controller.mousebutton_to_label.clear();
	controller.label_to_gamepad.clear();
	controller.tag_to_label.clear();
	for (auto& tag : controller.tags) {
		auto in_key = controls["controls"][tag]["keyboard_key"].as_string();
		auto in_button = controls["controls"][tag]["mouse_button"].as_string();
		auto in_gamepad = controls["controls"][tag]["gamepad_button"].as<int>();
		if (controller.string_to_key.contains(in_key)) { controller.key_to_label.insert({controller.string_to_key.at(in_key), tag}); }
		if (controller.string_to_mousebutton.contains(in_button)) { controller.mousebutton_to_label.insert({controller.string_to_mousebutton.at(in_button), tag}); }
		if (in_gamepad != -1) { controller.label_to_gamepad.insert({tag, in_gamepad}); }
		if (in_button.empty()) {
			controller.tag_to_label.insert({tag, in_key});
		} else {
			controller.tag_to_label.insert({tag, in_button});
		}
	}
}

void DataManager::save_controls(config::ControllerMap& controller) { controls.dj::Json::to_file((finder.resource_path + "/data/config/control_map.json").c_str()); }

void DataManager::reset_controls() { controls = dj::Json::from_file((finder.resource_path + "/data/config/defaults.json").c_str()); }

int DataManager::get_room_index(int id) {
	auto ctr{0};
	for (auto& room : rooms) {
		if (room == id) { return ctr; }
		++ctr;
	}
	return ctr;
}

std::vector<world::Layer>& DataManager::get_layers(int id) { return map_layers.at(get_room_index(id)); }

} // namespace data
