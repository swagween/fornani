
#include "fornani/automa/states/Dojo.hpp"
#include <fornani/systems/Event.hpp>
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/utils/Random.hpp"

namespace fornani::automa {

static bool item_acquisition{};
static bool gun_acquisition{};
static bool item_music_played{};
static bool b_reveal_item{};
static bool b_read_item{};
static int item_modifier{};

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
static void trigger_reveal_item(int to) {
	b_reveal_item = true;
	item_modifier = to;
}

Dojo::Dojo(ServiceProvider& svc, player::Player& player, std::string_view scene, int room_number, std::string_view room_name) : GameState(svc, player, scene, room_number), map(svc, player), gui_map(svc, player), m_services(&svc) {

	// register game events
	svc.events.register_event(std::make_unique<Event<int, int>>("GivePlayerItem", std::bind(&player::Player::give_item_by_id, &player, std::placeholders::_1, std::placeholders::_2)));
	svc.events.register_event(std::make_unique<Event<int>>("RevealItem", &trigger_reveal_item));
	svc.events.register_event(std::make_unique<Event<int>>("ReadItem", &trigger_read_item));
	svc.events.register_event(std::make_unique<Event<int>>("AcquireItem", &trigger_item));
	svc.events.register_event(std::make_unique<Event<int>>("AcquireGun", &trigger_gun));

	NANI_LOG_DEBUG(m_logger, "test: {}", svc.data.item_label_from_id(7));

	svc.menu_controller.reset_vendor_dialog();
	open_vendor = false;
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
		map.load(svc, room_number);
		bake_maps(svc, {map.room_id}, true);
		bake_maps(svc, svc.data.rooms);
	}
	hud.orient(svc, player); // reset hud position to corner
	svc.soundboard.turn_on();

	player.set_camera_bounds(map.real_dimensions);
	player.force_camera_center();

	// TODO: refactor player initialization
	player.collider.physics.zero();

	bool found_one{};
	// only search for door entry if room was not loaded from main menu and player didn't die
	if (!svc.state_controller.actions.test(Actions::save_loaded) && !svc.state_controller.actions.test(Actions::player_death)) {
		for (auto& portal : map.portals) {
			if (portal.get_destination() == svc.state_controller.source_id) {
				found_one = true;
				sf::Vector2f spawn_position{portal.get_world_position().x + (portal.get_world_dimensions().x * 0.5f), portal.get_world_position().y + portal.get_world_dimensions().y - player.height()};
				player.set_position(spawn_position, true);
				player.force_camera_center();
				if (portal.activate_on_contact() && portal.is_left_or_right()) {
					enter_room.start(90);
				} else {
					if (!portal.already_open()) { portal.close(); }
					player.set_idle();
				}
				if (portal.is_bottom()) {
					player.collider.physics.acceleration.y = -player.physics_stats.jump_velocity;
					player.collider.physics.acceleration.x = player.controller.facing_left() ? -player.physics_stats.x_acc : player.physics_stats.x_acc;
				}
			}
		}
	}
	if (!found_one) {
		float ppx = svc.data.get_save()["player_data"]["position"]["x"].as<float>();
		float ppy = svc.data.get_save()["player_data"]["position"]["y"].as<float>();
		sf::Vector2f player_pos = {ppx, ppy};
		player.set_position(player_pos);
	}

	if (player.piggybacker) { player.piggybacker.value().set_position(player.collider.physics.position); }

	// save was loaded from a json, or player died, so we successfully skipped door search
	svc.state_controller.actions.reset(Actions::save_loaded);
	if (!player.is_dead()) { svc.state_controller.actions.reset(Actions::player_death); }
	player.visit_history.push_room(room_number);

	player.controller.prevent_movement();
	loading.start();
}

void Dojo::tick_update(ServiceProvider& svc, capo::IEngine& engine) {
	GameState::tick_update(svc, engine);

	// handle events
	if (item_acquisition) { acquire_item(svc, *player, item_modifier); }
	if (gun_acquisition) { acquire_gun(svc, *player, item_modifier); }
	if (b_read_item) { read_item(item_modifier); }
	if (b_reveal_item) {
		player->catalog.inventory.reveal_item(item_modifier);
		b_reveal_item = false;
	}
	if (!m_console && item_music_played) {
		svc.music_player.resume();
		item_music_played = false;
	}

	// gamepad disconnected
	if (svc.controller_map.process_gamepad_disconnection()) { pause_window = std::make_unique<gui::PauseWindow>(svc); }

	svc.a11y.set_action_ctx_bar_enabled(false);

	loading.is_complete() && !vendor_dialog ? svc.app_flags.set(AppFlags::in_game) : svc.app_flags.reset(AppFlags::in_game);
	loading.update();

	svc.soundboard.play_sounds(engine, svc, map.get_echo_count(), map.get_echo_rate());

	// set action set
	if (pause_window || m_console) {
		svc.controller_map.set_action_set(config::ActionSet::Menu);
		svc.controller_map.set_joystick_throttle({});
	} else if (inventory_window) {
		svc.controller_map.set_action_set(config::ActionSet::Inventory);
	} else {
		svc.controller_map.set_action_set(config::ActionSet::Platformer);
	}

	if (pause_window) {
		if (m_console) { m_console.value()->update(svc); }
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
		return;
	}

	if (m_console) {
		if (m_console.value()->just_began()) {
			player->wardrobe_widget.update(*player);
			m_console.value()->set_nani_sprite(player->wardrobe_widget.get_sprite());
		}
		m_console.value()->update(svc);
	}

	svc.world_clock.update(svc);

	if (inventory_window && !m_console) {
		inventory_window.value()->update(svc, *player, map);
		if (inventory_window.value()->exit_requested()) {
			inventory_window = {};
			svc.music_player.resume();
		}
		return;
	}

	// TODO: move this somehwere else
	if (vendor_dialog) {
		if (open_vendor) {
			map.transition.end();
			open_vendor = false;
		}
		map.transition.update(*player);
		vendor_dialog.value()->update(svc, map, *player);
		if (!vendor_dialog.value()->is_open()) {
			if (vendor_dialog.value()->made_profit()) { svc.soundboard.flags.item.set(audio::Item::orb_max); }
			vendor_dialog = {};
			svc.soundboard.flags.menu.set(audio::Menu::backward_switch);
		}
		return;
	}

	if (!m_console) {
		if (svc.menu_controller.vendor_dialog_opened()) {
			map.transition.start();
			open_vendor = true;
		}
		if (open_vendor && map.transition.is_done()) {
			vendor_dialog = std::make_unique<gui::VendorDialog>(svc, map, *player, svc.menu_controller.get_menu_id());
			svc.controller_map.set_action_set(config::ActionSet::Menu);
			svc.soundboard.flags.console.set(audio::Console::menu_open);
		}
	}

	// TODO: re-do this once I reimplement regular inventory + collectible items. we will check collectible_items_view, or something else.
	if (player->visit_history.traveled_far() || svc.data.marketplace.at(3).inventory.items_view().empty()) {
		util::random::set_vendor_seed();
		for (auto& vendor : svc.data.marketplace) { vendor.second.generate_inventory(svc); }
		player->visit_history.clear();
	}

	// in-game menus
	if (svc.controller_map.digital_action_status(config::DigitalAction::platformer_open_inventory).triggered) { inventory_window = std::make_unique<gui::InventoryWindow>(svc, gui_map, *player); }
	if (svc.controller_map.digital_action_status(config::DigitalAction::platformer_toggle_pause).triggered) { pause_window = std::make_unique<gui::PauseWindow>(svc); }

	enter_room.update();
	if (!m_console && svc.state_controller.actions.test(Actions::main_menu)) { svc.state_controller.actions.set(Actions::trigger); }
	if (enter_room.running()) { player->controller.autonomous_walk(); }

	player->update(map);
	map.update(svc, m_console);

	map.debug_mode = debug_mode;

	player->controller.clean();
	player->flags.triggers = {};

	map.background->update(svc);
	hud.update(svc, *player);
}

void Dojo::frame_update(ServiceProvider& svc) {}

void Dojo::render(ServiceProvider& svc, sf::RenderWindow& win) {
	map.render_background(svc, win, player->get_camera_position());
	map.render(svc, win, player->get_camera_position());

	if (!svc.greyblock_mode() && !svc.hide_hud()) { hud.render(svc, *player, win); }
	if (vendor_dialog) { vendor_dialog.value()->render(svc, win, *player, map); }
	if (inventory_window) { inventory_window.value()->render(svc, win, *player); }
	map.soft_reset.render(win);
	map.transition.render(win);
	map.bed_transition.render(win);
	if (pause_window) { pause_window.value()->render(svc, win); }
	if (m_console) {
		m_console.value()->render(win);
		m_console.value()->write(win);
	}
	if (svc.debug_mode()) { map.debug(); }
}

void Dojo::bake_maps(ServiceProvider& svc, std::vector<int> ids, bool current) {
	for (auto const& id : ids) {
		if (id == 0) { continue; } // intro
		gui_map.clear();
	}
}

void Dojo::acquire_item(ServiceProvider& svc, player::Player& player, int modifier) {
	player.give_item_by_id(modifier, 1);
	m_console = std::make_unique<gui::Console>(svc, svc.text.basic, "chest", gui::OutputType::no_skip);
	m_console.value()->display_item(modifier);
	m_console.value()->append(player.catalog.inventory.item_view(modifier).get_title());
	m_console.value()->append("!");
	svc.music_player.quick_play(svc.finder, "discovery");
	item_acquisition = false;
	item_music_played = true;
}

void Dojo::acquire_gun(ServiceProvider& svc, player::Player& player, int modifier) {
	auto tag = svc.data.get_gun_tag_from_id(modifier);
	if (!tag) { return; }
	NANI_LOG_DEBUG(m_logger, "Gun Tag: {}", tag->data());
	player.push_to_loadout(tag.value());
	m_console = std::make_unique<gui::Console>(svc, svc.text.basic, "chest", gui::OutputType::no_skip);
	m_console.value()->display_gun(modifier);
	m_console.value()->append(player.arsenal.value().get_weapon_at(tag.value()).get_label());
	m_console.value()->append("!");
	svc.music_player.quick_play(svc.finder, "discovery");
	gun_acquisition = false;
	item_music_played = true;
}

void Dojo::read_item(int id) {
	m_console = std::make_unique<gui::Console>(*m_services, m_services->text.item, m_services->data.item_label_from_id(id), gui::OutputType::gradual);
	b_read_item = false;
}

} // namespace fornani::automa
