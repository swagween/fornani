
#include <fornani/automa/states/Dojo.hpp>
#include <fornani/events/GameplayEvent.hpp>
#include <fornani/events/InventoryEvent.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/utils/Random.hpp>

namespace fornani::automa {

Dojo::Dojo(ServiceProvider& svc, player::Player& player, std::string_view scene, int room_number, std::string_view room_name) : GameplayState(svc, player, scene, room_number), m_enter_room{100}, m_loading{4} {

	m_map = world::Map{svc, player};

	m_type = StateType::dojo;
	player.set_flag(player::PlayerFlags::trial, false);

	// inventory events// inventory events
	svc.events.acquire_item_from_console_event.attach_to(p_slot, &Dojo::acquire_item_from_console, this);
	svc.events.acquire_item_event.attach_to(p_slot, &Dojo::acquire_item, this);
	// inventory events
	svc.events.acquire_item_from_console_event.attach_to(p_slot, &Dojo::acquire_item_from_console, this);
	svc.events.read_item_by_id_event.attach_to(p_slot, &Dojo::read_item, this);
	svc.events.equip_item_by_id_event.attach_to(p_slot, &Dojo::equip_item, this);
	svc.events.acquire_item_event.attach_to(p_slot, &Dojo::acquire_item, this);
	svc.events.acquire_weapon_event.attach_to(p_slot, &Dojo::acquire_gun, this);
	svc.events.acquire_weapon_from_console_event.attach_to(p_slot, &Dojo::acquire_gun_from_console, this);
	svc.events.remove_weapon_by_id_event.attach_to(p_slot, &Dojo::remove_gun_by_id, this);

	// gameplay events
	svc.events.open_vendor_event.attach_to(p_slot, &Dojo::open_vendor, this);
	svc.events.launch_cutscene_event.attach_to(p_slot, &Dojo::launch_cutscene, this);
	svc.events.add_map_marker_event.attach_to(p_slot, &Dojo::add_map_marker, this);

	m_map_markers.insert({1, "main"});
	m_map_markers.insert({2, "woodshine"});

	// create shaders
	p_world_shader = LightShader(svc.finder);
	p_gui_shader = LightShader(svc.finder);

	reload(svc, room_number);

	NANI_LOG_INFO(m_logger, "New Dojo instance created.");
}

void Dojo::tick_update(ServiceProvider& svc, capo::IEngine& engine) {

	// handle events
	if (svc.music_player.is_finished_playing() && m_flags.test(GameplayFlags::item_music_played)) {
		NANI_LOG_DEBUG(m_logger, "set fade in!");
		svc.music_player.resume();
		svc.music_player.fade_in(util::Sec{1.f});
		m_flags.reset(GameplayFlags::item_music_played);
	}

	svc.a11y.set_action_ctx_bar_enabled(svc.data.settings["tutorial"].as_bool());

	m_loading.is_complete() && !p_vendor_dialog ? svc.app_flags.set(AppFlags::in_game) : svc.app_flags.reset(AppFlags::in_game);
	m_loading.update();
	if (m_inspect_hint) {
		m_inspect_hint->update();
		if (m_inspect_hint->is_almost_complete()) {
			svc.notifications.push_notification(svc, svc.data.gui_text["notifications"]["inspect_hint"].as_string());
			svc.notifications.get_latest().insert_input_hint(svc, 10, 7);
			m_inspect_hint->start();
		}
	}
	if (svc.input_system.digital(input::DigitalAction::inspect).triggered) {
		svc.data.get_file().flags.reset(io::FileFlags::inspect_hint);
		m_inspect_hint.reset();
	}

	if (m_shoot_hint) {
		m_shoot_hint->update();
		if (m_shoot_hint->is_almost_complete()) {
			m_shoot_hint.reset();
			svc.notifications.push_notification(svc, svc.data.gui_text["notifications"]["shoot_hint"].as_string());
			svc.notifications.get_latest().insert_input_hint(svc, 5, 7);
		}
	}

	if (m_flags.consume(GameplayFlags::give_item)) {
		player->give_item(m_item_tag, 1);
		if (!m_console) {
			m_console = std::make_unique<gui::Console>(svc, svc.text.basic, "chest", gui::OutputType::no_skip);
			m_console.value()->append(player->catalog.inventory.find_item(m_item_tag)->get_title());
			m_console.value()->append("!");
		}
		m_console.value()->display_item(m_item_tag);
		svc.music_player.quick_play(svc.finder, "discovery");
		m_flags.set(GameplayFlags::item_music_played);
	}

	svc.soundboard.set_listener_position(player->get_ear_position());

	set_flag(GameplayStateFlags::early_tick_return, false);
	GameplayState::tick_update(svc, engine);
	if (has_flag_set(GameplayStateFlags::early_tick_return)) { return; }

	if (m_console) {
		m_flags.set(GameplayFlags::console_running);
		if (m_console.value()->was_response_created() && !m_console.value()->has_nani_portrait()) {
			player->wardrobe_widget.update(*player);
			m_console.value()->set_nani_sprite(player->wardrobe_widget.get_sprite());
		}
	} else if (m_flags.consume(GameplayFlags::console_running)) {
		auto to_set = p_inventory_window || p_vendor_dialog ? input::ActionSet::Menu : input::ActionSet::Platformer;
		svc.input_system.set_action_set(to_set);
	}

	svc.world_clock.update(svc);

	if (p_inventory_window) {
		m_map->set_target_balance(0.f, audio::BalanceTarget::music);
		m_map->set_target_balance(0.f, audio::BalanceTarget::ambience);
		m_map->update_balance(svc);
		m_map->transition.update(*player);
		if (!m_console) {
			p_inventory_window.value()->update(svc, *player, *m_map);
			if (p_inventory_window.value()->exit_requested()) {
				p_inventory_window = {};
				svc.input_system.set_action_set(input::ActionSet::Platformer);
			}
			return;
		}
	}

	if (check_for_vendor(svc)) { return; }

	// TODO: re-do this once I reimplement regular inventory + collectible items. we will check collectible_items_view, or something else.
	if (player->visit_history.traveled_far() || svc.data.marketplace.at(3).inventory.items_view().is_empty()) {
		random::set_vendor_seed();
		for (auto& vendor : svc.data.marketplace) { vendor.second.generate_inventory(svc); }
		player->visit_history.clear();
	}

	// in-game menus
	if (svc.input_system.digital(input::DigitalAction::inventory).triggered) { p_inventory_window = std::make_unique<gui::InventoryWindow>(svc, *m_map, *player); }

	m_enter_room.update();
	if (m_enter_room.running()) { player->controller.autonomous_walk(); }

	// physical tick
	player->update(*m_map);
	player->start_tick();
	m_map->update(svc, m_console);
	handle_player_death(svc, *player);

	m_map->debug_mode = debug_mode;

	player->end_tick();
	if (!m_console) { player->set_busy(false); }

	m_map->background->update(svc);
	hud.update(svc, *player);
}

void Dojo::frame_update(ServiceProvider& svc) {}

void Dojo::render(ServiceProvider& svc, sf::RenderWindow& win) {

	if (!m_map) { return; }

	// TODO: do this somewhere else
	if (p_world_shader) {
		m_map->render_background(svc, win, p_world_shader, player->get_camera_position());
		m_map->render(svc, win, p_world_shader, player->get_camera_position());
		p_world_shader->clear_point_lights();

		float aspect = m_map->real_dimensions.x / m_map->real_dimensions.y;
		for (auto& pl : m_map->point_lights) {
			auto uv = pl.get_position().componentWiseDiv(m_map->real_dimensions);
			auto normalized = sf::Vector2f{(uv.x - 0.5f) * aspect + 0.5f, uv.y};
			pl.position = normalized;
			p_world_shader->add_point_light(pl);
		}

		auto puv = player->get_lantern_position().componentWiseDiv(m_map->real_dimensions);
		auto normalized = sf::Vector2f{(puv.x - 0.5f) * aspect + 0.5f, puv.y};
		auto ppl = PointLight(svc.data.light["lantern"], puv);
		ppl.position = normalized;
		if (player->has_item_equipped(svc.data.item_id_from_label("lantern"))) { p_world_shader->add_point_light(ppl); }
		// m_shader->debug();
	}

	GameplayState::render(svc, win);
}

void Dojo::reload(ServiceProvider& svc, int target_state) {
	m_map->clear();
	m_flags.reset(GameplayFlags::transitioning);
	m_map->transition.set(graphics::TransitionState::black);
	m_map->transition.hang();
	svc.menu_controller.reset_vendor_dialog();
	if (!svc.data.is_room_discovered(target_state)) {
		svc.data.discovered_rooms.add(target_state);
		svc.stats.world.rooms_discovered.update();
	}
	player->reset_flags();

	// the following should only happen for the editor demo
	if (!svc.data.exists(target_state)) {
		svc.data.rooms.push_back(target_state);
		svc.data.load_data();
	} else {
		m_map->load(svc, m_console, target_state);
		NANI_LOG_INFO(m_logger, "Map loaded.");
	}

	hud.reset_position(); // reset hud position to corner
	svc.soundboard.turn_on();
	player->set_camera_bounds(m_map->real_dimensions);
	player->force_camera_center();

	// TODO: refactor player initialization
	player->get_collider().physics.zero();

	bool found_one{};
	// only search for door entry if room was not loaded from main menu and player didn't die
	if (!svc.state_controller.actions.test(Actions::save_loaded) && !svc.state_controller.actions.test(Actions::player_death)) { found_one = m_map->handle_entry(*player, m_enter_room); }
	if (!found_one) {
		float ppx = svc.data.get_save()["player_data"]["position"]["x"].as<float>();
		float ppy = svc.data.get_save()["player_data"]["position"]["y"].as<float>();
		sf::Vector2f player_pos = {ppx, ppy};
		svc.demo_mode() ? player->place_at_demo_position() : player->set_position(player_pos);
	}

	// save was loaded from a json, or player died, so we successfully skipped door search
	svc.state_controller.actions.reset(Actions::save_loaded);
	if (!player->is_dead()) { svc.state_controller.actions.reset(Actions::player_death); }
	player->visit_history.push_room(target_state);

	player->controller.prevent_movement();
	m_loading.start();
	p_world_shader->set_darken(m_map->darken_factor);
	p_world_shader->set_texture_size(m_map->real_dimensions / constants::f_scale_factor);
	p_gui_shader->set_texture_size(svc.window->f_screen_dimensions() * 3.f); // 3 is the number of screen-sized "cells" in the inventory window
	svc.app_flags.reset(automa::AppFlags::custom_map_start);

	if (svc.data.get_file().flags.test(io::FileFlags::inspect_hint)) {
		m_inspect_hint = util::Cooldown{2000};
		m_inspect_hint->start();
	}

	m_flags.reset(GameplayFlags::death_console_launched);
}

void Dojo::acquire_item_from_console(ServiceProvider& svc, int id) { acquire_item(svc, svc.data.item_label_from_id(id)); }

void Dojo::acquire_item(ServiceProvider& svc, std::string_view tag) {
	m_item_tag = tag.data();
	m_flags.set(GameplayFlags::give_item);
}

void Dojo::acquire_gun_from_console(ServiceProvider& svc, int id) {
	if (auto tag = svc.data.get_gun_tag_from_id(id)) { acquire_gun(svc, *tag); }
}

void Dojo::acquire_gun(ServiceProvider& svc, std::string_view tag) {
	if (tag == "bryns_gun") {
		m_shoot_hint = util::Cooldown{500};
		m_shoot_hint->start();
	}
	NANI_LOG_DEBUG(m_logger, "Gun Tag: {}", tag.data());
	player->push_to_loadout(tag);
	if (!m_console) {
		m_console = std::make_unique<gui::Console>(svc, svc.text.basic, "chest", gui::OutputType::no_skip);
		m_console.value()->append(player->arsenal.value().get_weapon_at(tag).get_label());
		m_console.value()->append("!");
	}
	m_console.value()->display_gun(tag);
	svc.music_player.quick_play(svc.finder, "revelation");

	m_flags.set(GameplayFlags::item_music_played);
}

void Dojo::remove_gun_by_id(ServiceProvider& svc, int id) {
	if (auto tag = svc.data.get_gun_tag_from_id(id)) { remove_gun(svc, *tag); }
}

void Dojo::equip_item(ServiceProvider& svc, int id) {
	auto equipped = player->equip_item(id);
	equipped == player::EquipmentStatus::equipped	  ? svc.soundboard.flags.item.set(audio::Item::equip)
	: equipped == player::EquipmentStatus::unequipped ? svc.soundboard.flags.menu.set(audio::Menu::backward_switch)
													  : svc.soundboard.flags.menu.set(audio::Menu::error);
	auto tag = equipped == player::EquipmentStatus::equipped ? "equipped" : "unequipped";
	auto qualifier = svc.data.gui_text["notifications"][tag].as_string();
	auto message = equipped != player::EquipmentStatus::failure ? qualifier + std::string{svc.data.item_label_from_id(id)} + "." : svc.data.gui_text["notifications"]["slots_full"].as_string();
	svc.notifications.push_notification(svc, message);
}

void Dojo::open_vendor(ServiceProvider& svc, int id) {
	m_flags.set(GameplayFlags::open_vendor);
	m_vendor_id = id;
}

void Dojo::launch_cutscene(ServiceProvider& svc, int id) {
	if (m_console) { return; }
	m_map->cutscene_catalog.push_cutscene(svc, *m_map, *player, id);
}

void Dojo::add_map_marker(ServiceProvider& svc, int room_id, int type, int questline) {
	if (!m_map) { return; }
	if (m_map_markers.contains(questline)) {
		svc.quest_table.set_quest_progression("map_markers", Subquest{m_map_markers.at(questline), room_id}, 1, {});
		svc.notifications.push_notification(svc, svc.data.gui_text["notifications"]["map_marker"].as_string());
	}
}

bool Dojo::check_for_vendor(ServiceProvider& svc) {
	if (m_console) { return false; }
	if (m_flags.test(GameplayFlags::open_vendor)) {
		if (m_map->transition.is(graphics::TransitionState::inactive)) {
			m_map->transition.start();
			NANI_LOG_DEBUG(m_logger, "Vendor Started");
		}
		if (m_map->transition.is(graphics::TransitionState::black)) {
			m_map->transition.end();
			NANI_LOG_DEBUG(m_logger, "Vendor Opened");
			p_vendor_dialog = std::make_unique<gui::VendorDialog>(svc, *m_map, *player, m_vendor_id);
			svc.input_system.set_action_set(input::ActionSet::Menu);
			svc.soundboard.flags.console.set(audio::Console::menu_open);
			m_flags.reset(GameplayFlags::open_vendor);
		}
	}
	if (p_vendor_dialog) {
		m_map->transition.update(*player);
		p_vendor_dialog.value()->update(svc, *m_map, *player);
		if (!p_vendor_dialog.value()->is_open()) {
			if (p_vendor_dialog.value()->made_profit()) { svc.soundboard.flags.item.set(audio::Item::orb_max); }
			p_vendor_dialog = {};
			svc.input_system.set_action_set(input::ActionSet::Platformer);
			if (p_gui_shader) { p_gui_shader->set_darken(0.f); }
		}
		m_map->set_target_balance(0.f, audio::BalanceTarget::music);
		m_map->set_target_balance(0.f, audio::BalanceTarget::ambience);
		m_map->update_balance(svc);
		return true;
	}
	return false;
}

void Dojo::remove_gun(ServiceProvider& svc, std::string_view tag) {
	auto label = player->arsenal.value().get_weapon_at(tag).get_label();
	svc.notifications.push_notification(svc, svc.data.gui_text["notifications"]["removed"].as_string() + std::string{label} + ".");
	if (!m_console) {
		m_console = std::make_unique<gui::Console>(svc, svc.text.basic, "removed", gui::OutputType::no_skip);
		m_console.value()->append(player->arsenal.value().get_weapon_at(tag).get_label());
		m_console.value()->append(".");
	}
	player->pop_from_loadout(tag);
	svc.soundboard.flags.item.set(audio::Item::unequip);
	m_console.value()->display_gun(tag, false);
}

void Dojo::read_item(int id) { m_console = std::make_unique<gui::Console>(*p_services, p_services->text.item, p_services->data.item_label_from_id(id), gui::OutputType::gradual); }

void Dojo::handle_player_death(ServiceProvider& svc, player::Player& player) {
	if (!m_map) { return; }
	if (!player.is_dead()) { return; }

	// check if player died
	if (!m_flags.test(GameplayFlags::game_over) && player.is_death_complete()) {
		m_flags.set(GameplayFlags::death_console_launched);
		svc.app_flags.reset(automa::AppFlags::in_game);
		m_console = std::make_unique<gui::Console>(svc, svc.text.basic, "death", gui::OutputType::gradual, static_cast<int>(player.get_i_death_type()));
		m_flags.set(GameplayFlags::game_over);
		svc.music_player.load(svc.finder, "mortem");
		svc.music_player.play_looped();
		svc.soundboard.turn_off();
		svc.stats.player.death_count.update();
	}

	if (!m_console && m_flags.test(GameplayFlags::game_over)) {
		if (!m_flags.test(GameplayFlags::transitioning)) { m_map->transition.start(); }
		m_flags.set(GameplayFlags::transitioning);
		if (m_map->transition.is(graphics::TransitionState::black) && !svc.state_controller.actions.test(automa::Actions::player_death)) {
			svc.state_controller.actions.set(automa::Actions::player_death);
			m_flags.reset(GameplayFlags::game_over);
		}
	}
}

} // namespace fornani::automa
