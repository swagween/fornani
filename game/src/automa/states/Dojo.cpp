
#include "fornani/automa/states/Dojo.hpp"
#include <fornani/systems/Event.hpp>
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/utils/Random.hpp"

namespace fornani::automa {

static bool item_acquisition{};
static bool gun_acquisition{};
static bool gun_removal{};
static bool item_music_played{};
static bool b_reveal_item{};
static bool b_read_item{};
static bool b_equip_item{};
static bool b_play_song{};
static bool b_open_vendor{};
static int item_modifier{};
static int vendor_id{};
static int song_id{};

static void trigger_item(int to) {
	item_acquisition = true;
	item_modifier = to;
}
static void trigger_gun(int to) {
	gun_acquisition = true;
	item_modifier = to;
}
static void trigger_read_item(int to) {
	b_read_item = true;
	item_modifier = to;
}
static void trigger_equip_item(int to) {
	b_equip_item = true;
	item_modifier = to;
}
static void trigger_reveal_item(int to) {
	b_reveal_item = true;
	item_modifier = to;
}
static void trigger_song(int to) {
	b_play_song = true;
	song_id = to;
}
static void trigger_remove_gun(int which) {
	gun_removal = true;
	item_modifier = which;
}
static void open_vendor(int which) {
	b_open_vendor = true;
	vendor_id = which;
}

Dojo::Dojo(ServiceProvider& svc, player::Player& player, std::string_view scene, int room_number, std::string_view room_name)
	: GameState(svc, player, scene, room_number), map(svc, player), m_services(&svc), m_enter_room{100}, m_loading{4} {

	m_type = StateType::game;

	// register game events
	svc.events.register_event(std::make_unique<Event<int, int>>("GivePlayerItem", std::bind(&player::Player::give_item_by_id, &player, std::placeholders::_1, std::placeholders::_2)));
	svc.events.register_event(std::make_unique<Event<int>>("RevealItem", &trigger_reveal_item));
	svc.events.register_event(std::make_unique<Event<int>>("ReadItem", &trigger_read_item));
	svc.events.register_event(std::make_unique<Event<int>>("EquipItem", &trigger_equip_item));
	svc.events.register_event(std::make_unique<Event<int>>("AcquireItem", &trigger_item));
	svc.events.register_event(std::make_unique<Event<int>>("AcquireGun", &trigger_gun));
	svc.events.register_event(std::make_unique<Event<int>>("PlaySong", &trigger_song));
	svc.events.register_event(std::make_unique<Event<int>>("RemovePlayerWeapon", &trigger_remove_gun));
	svc.events.register_event(std::make_unique<Event<int>>("OpenVendor", &open_vendor));

	// create shaders
	m_world_shader = LightShader(svc.finder);
	m_gui_shader = LightShader(svc.finder);

	svc.menu_controller.reset_vendor_dialog();
	if (!svc.data.room_discovered(room_number)) {
		svc.data.discovered_rooms.push_back(room_number);
		svc.stats.world.rooms_discovered.update();
	}
	player.reset_flags();
	// the following should only happen for the editor demo
	if (!svc.data.exists(room_number)) {
		svc.data.rooms.push_back(room_number);
		svc.data.load_data(room_name.data());
	} else {
		map.load(svc, m_console, room_number);
		NANI_LOG_INFO(m_logger, "Map loaded.");
	}

	hud.orient(svc, player); // reset hud position to corner
	svc.soundboard.turn_on();

	player.set_camera_bounds(map.real_dimensions);
	player.force_camera_center();

	// TODO: refactor player initialization
	player.get_collider().physics.zero();

	bool found_one{};
	// only search for door entry if room was not loaded from main menu and player didn't die
	if (!svc.state_controller.actions.test(Actions::save_loaded) && !svc.state_controller.actions.test(Actions::player_death)) { found_one = map.handle_entry(player, m_enter_room); }
	if (!found_one) {
		float ppx = svc.data.get_save()["player_data"]["position"]["x"].as<float>();
		float ppy = svc.data.get_save()["player_data"]["position"]["y"].as<float>();
		sf::Vector2f player_pos = {ppx, ppy};
		player.set_position(player_pos);
	}

	// save was loaded from a json, or player died, so we successfully skipped door search
	svc.state_controller.actions.reset(Actions::save_loaded);
	if (!player.is_dead()) { svc.state_controller.actions.reset(Actions::player_death); }
	player.visit_history.push_room(room_number);

	player.controller.prevent_movement();
	m_loading.start();
	m_world_shader->set_darken(map.darken_factor);
	m_world_shader->set_texture_size(map.real_dimensions / constants::f_scale_factor);
	m_gui_shader->set_texture_size(svc.window->f_screen_dimensions() * 3.f); // 3 is the number of screen-sized "cells" in the inventory window
	svc.app_flags.reset(automa::AppFlags::custom_map_start);

	NANI_LOG_INFO(m_logger, "New Dojo instance created.");
}

void Dojo::tick_update(ServiceProvider& svc, capo::IEngine& engine) {
	// handle events
	if (item_acquisition) { acquire_item(svc, *player, item_modifier); }
	if (gun_acquisition) { acquire_gun(svc, *player, item_modifier); }
	if (gun_removal) { remove_gun(svc, *player, item_modifier); }
	if (b_play_song) {
		svc.music_player.play_song_by_id(svc.finder, song_id);
		b_play_song = false;
	}
	if (b_read_item) { read_item(item_modifier); }
	if (b_equip_item) {
		auto equipped = player->equip_item(item_modifier);
		equipped == player::EquipmentStatus::equipped	  ? svc.soundboard.flags.item.set(audio::Item::equip)
		: equipped == player::EquipmentStatus::unequipped ? svc.soundboard.flags.menu.set(audio::Menu::backward_switch)
														  : svc.soundboard.flags.menu.set(audio::Menu::error);
		auto tag = equipped == player::EquipmentStatus::equipped ? "equipped" : "unequipped";
		auto qualifier = svc.data.gui_text["notifications"][tag].as_string();
		auto message = equipped != player::EquipmentStatus::failure ? qualifier + std::string{svc.data.item_label_from_id(item_modifier)} + "." : svc.data.gui_text["notifications"]["slots_full"].as_string();
		svc.notifications.push_notification(svc, message);
		b_equip_item = false;
	}
	if (b_reveal_item) {
		player->catalog.inventory.reveal_item(item_modifier);
		b_reveal_item = false;
	}
	if (!m_console && item_music_played) {
		svc.music_player.resume();
		item_music_played = false;
	}

	// gamepad disconnected
	if (svc.controller_map.process_gamepad_disconnection()) { pause_window = std::make_unique<gui::PauseWindow>(svc, std::vector<std::string>{"resume", "settings", "controls", "quit"}); }

	svc.a11y.set_action_ctx_bar_enabled(false);

	m_loading.is_complete() && !vendor_dialog ? svc.app_flags.set(AppFlags::in_game) : svc.app_flags.reset(AppFlags::in_game);
	m_loading.update();

	// set action set
	if (pause_window || m_console) {
		svc.controller_map.set_action_set(config::ActionSet::Menu);
		svc.controller_map.set_joystick_throttle({});
	} else if (inventory_window || vendor_dialog) {
		svc.controller_map.set_action_set(config::ActionSet::Inventory);
	} else {
		svc.controller_map.set_action_set(config::ActionSet::Platformer);
	}

	if (pause_window) {
		pause_window.value()->update(svc, m_console);
		if (pause_window.value()->settings_requested()) {
			flags.set(GameStateFlags::settings_request);
			pause_window.value()->reset();
		}
		if (pause_window.value()->controls_requested()) {
			flags.set(GameStateFlags::controls_request);
			pause_window.value()->reset();
		}
		if (pause_window.value()->exit_requested()) { pause_window = {}; }
		GameState::tick_update(svc, engine);
		return;
	}
	GameState::tick_update(svc, engine);

	if (m_console) {
		if (m_console.value()->was_response_created() && !m_console.value()->has_nani_portrait()) {
			player->wardrobe_widget.update(*player);
			m_console.value()->set_nani_sprite(player->wardrobe_widget.get_sprite());
		}
	}

	svc.world_clock.update(svc);

	if (inventory_window && !m_console) {
		inventory_window.value()->update(svc, *player, map);
		if (inventory_window.value()->exit_requested()) { inventory_window = {}; }
		svc.ambience_player.set_balance(svc.music_player.get_fade().get_normalized());
		return;
	} else {
		svc.ambience_player.set_balance(map.cooldowns.fade_obscured.get_normalized());
	}

	// TODO: move this somehwere else
	if (vendor_dialog) {
		map.transition.update(*player);
		vendor_dialog.value()->update(svc, map, *player);
		if (!vendor_dialog.value()->is_open()) {
			if (vendor_dialog.value()->made_profit()) { svc.soundboard.flags.item.set(audio::Item::orb_max); }
			vendor_dialog = {};
			if (m_gui_shader) { m_gui_shader->set_darken(0.f); }
		}
		return;
	}

	if (!m_console) {
		if (b_open_vendor && map.transition.is(graphics::TransitionState::inactive)) {
			map.transition.start();
			NANI_LOG_DEBUG(m_logger, "Vendor Started");
		}
		if (b_open_vendor && map.transition.is(graphics::TransitionState::black)) {
			map.transition.end();
			NANI_LOG_DEBUG(m_logger, "Vendor Opened");
			vendor_dialog = std::make_unique<gui::VendorDialog>(svc, map, *player, vendor_id);
			svc.controller_map.set_action_set(config::ActionSet::Menu);
			svc.soundboard.flags.console.set(audio::Console::menu_open);
			b_open_vendor = false;
		}
	}

	// TODO: re-do this once I reimplement regular inventory + collectible items. we will check collectible_items_view, or something else.
	if (player->visit_history.traveled_far() || svc.data.marketplace.at(3).inventory.items_view().empty()) {
		random::set_vendor_seed();
		for (auto& vendor : svc.data.marketplace) { vendor.second.generate_inventory(svc); }
		player->visit_history.clear();
	}

	// in-game menus
	if (svc.controller_map.digital_action_status(config::DigitalAction::platformer_open_inventory).triggered) { inventory_window = std::make_unique<gui::InventoryWindow>(svc, map, *player); }
	if (svc.controller_map.digital_action_status(config::DigitalAction::platformer_toggle_pause).triggered) { pause_window = std::make_unique<gui::PauseWindow>(svc, std::vector<std::string>{"resume", "settings", "controls", "quit"}); }

	m_enter_room.update();
	if (!m_console && svc.state_controller.actions.test(Actions::main_menu)) { svc.state_controller.actions.set(Actions::trigger); }
	if (m_enter_room.running()) { player->controller.autonomous_walk(); }

	// physical tick
	player->update(map);
	player->start_tick();
	map.update(svc, m_console);

	map.debug_mode = debug_mode;

	player->end_tick();
	if (!m_console) { player->flags.state.reset(player::State::busy); }

	map.background->update(svc);
	hud.update(svc, *player);
}

void Dojo::frame_update(ServiceProvider& svc) {}

void Dojo::render(ServiceProvider& svc, sf::RenderWindow& win) {

	// TODO: do this somewhere else
	if (m_world_shader) {
		map.render_background(svc, win, m_world_shader, player->get_camera_position());
		map.render(svc, win, m_world_shader, player->get_camera_position());
		m_world_shader->clear_point_lights();

		float aspect = map.real_dimensions.x / map.real_dimensions.y;
		for (auto& pl : map.point_lights) {
			auto uv = pl.get_position().componentWiseDiv(map.real_dimensions);
			auto normalized = sf::Vector2f{(uv.x - 0.5f) * aspect + 0.5f, uv.y};
			pl.position = normalized;
			m_world_shader->add_point_light(pl);
		}

		auto puv = player->get_lantern_position().componentWiseDiv(map.real_dimensions);
		auto normalized = sf::Vector2f{(puv.x - 0.5f) * aspect + 0.5f, puv.y};
		auto ppl = PointLight(svc.data.light["lantern"], puv);
		ppl.position = normalized;
		if (player->has_item_equipped(svc.data.item_id_from_label("lantern"))) { m_world_shader->add_point_light(ppl); }
		// m_shader->debug();
	}

	m_console || svc.state_flags.test(automa::StateFlags::cutscene) ? svc.state_flags.set(automa::StateFlags::hide_hud) : svc.state_flags.reset(automa::StateFlags::hide_hud);

	if (!svc.greyblock_mode() && !svc.hide_hud()) { hud.render(svc, *player, win); }

	if (vendor_dialog && m_gui_shader) { vendor_dialog.value()->render(svc, win, *player, map, *m_gui_shader); }
	if (inventory_window && m_gui_shader) { inventory_window.value()->render(svc, win, *player, *m_gui_shader); }

	map.transition.render(win);
	if (pause_window) { pause_window.value()->render(svc, win); }
	if (m_console) {
		m_console.value()->render(win);
		m_console.value()->write(win);
	}
	if (svc.debug_mode()) { map.debug(); }

	svc.notifications.render(win);
}

void Dojo::acquire_item(ServiceProvider& svc, player::Player& player, int modifier) {
	player.give_item_by_id(modifier, 1);
	if (!m_console) {
		m_console = std::make_unique<gui::Console>(svc, svc.text.basic, "chest", gui::OutputType::no_skip);
		m_console.value()->append(player.catalog.inventory.item_view(modifier).get_title());
		m_console.value()->append("!");
	}
	m_console.value()->display_item(modifier);
	svc.music_player.quick_play(svc.finder, "discovery");
	item_acquisition = false;
	item_music_played = true;
}

void Dojo::acquire_gun(ServiceProvider& svc, player::Player& player, int modifier) {
	auto tag = svc.data.get_gun_tag_from_id(modifier);
	if (!tag) { return; }
	NANI_LOG_DEBUG(m_logger, "Gun Tag: {}", tag->data());
	player.push_to_loadout(tag.value());
	if (!m_console) {
		m_console = std::make_unique<gui::Console>(svc, svc.text.basic, "chest", gui::OutputType::no_skip);
		m_console.value()->append(player.arsenal.value().get_weapon_at(tag.value()).get_label());
		m_console.value()->append("!");
	}
	m_console.value()->display_gun(modifier);
	svc.music_player.quick_play(svc.finder, "discovery");
	gun_acquisition = false;
	item_music_played = true;
}

void Dojo::remove_gun(ServiceProvider& svc, player::Player& player, int modifier) {
	auto tag = svc.data.get_gun_tag_from_id(modifier);
	if (!tag) { return; }
	NANI_LOG_DEBUG(m_logger, "Gun Tag: {}", tag->data());
	auto label = player.arsenal.value().get_weapon_at(tag.value()).get_label();
	svc.notifications.push_notification(svc, "Removed " + std::string{label} + ".");
	if (!m_console) {
		m_console = std::make_unique<gui::Console>(svc, svc.text.basic, "removed", gui::OutputType::no_skip);
		m_console.value()->append(player.arsenal.value().get_weapon_at(tag.value()).get_label());
		m_console.value()->append(".");
	}
	player.pop_from_loadout(tag.value());
	svc.soundboard.flags.item.set(audio::Item::unequip);
	m_console.value()->display_gun(modifier);
	gun_removal = false;
}

void Dojo::read_item(int id) {
	m_console = std::make_unique<gui::Console>(*m_services, m_services->text.item, m_services->data.item_label_from_id(id), gui::OutputType::gradual);
	b_read_item = false;
}

} // namespace fornani::automa
