
#include <fornani/automa/states/Intro.hpp>
#include <fornani/events/GameplayEvent.hpp>
#include <fornani/service/ServiceProvider.hpp>

namespace fornani::automa {

Intro::Intro(ServiceProvider& svc, player::Player& player, std::string_view scene, int room_number)
	: GameplayState(svc, player, scene, room_number), m_airship{svc, "scenery_firstwind_airship", {480, 256}}, m_cloud_sea{svc, "cloud_sea"}, m_cloud{svc, "cloud"}, m_intro_shot{1600}, m_wait{800}, m_end_wait{800}, m_attack_fadeout{2600},
	  m_location_text{svc, svc.data.gui_text["locations"]["firstwind"].as_string_view()} {
	m_map = world::Map{svc, player};

	svc.music_player.load(svc.finder, "wind");
	svc.ambience_player.load(svc.finder, "firstwind");
	svc.ambience_player.play();
	m_wait.start();

	player.reset_flags();
	m_map->load(svc, m_console, room_number);
	m_map->transition.set_duration(400);

	svc.soundboard.turn_on();

	player.set_camera_bounds(m_map->real_dimensions);
	player.force_camera_center();

	player.set_position(sf::Vector2f{14.5f, 10.f} * constants::f_cell_size);

	m_world_shader = LightShader(svc.finder);
	svc.a11y.set_action_ctx_bar_enabled(false);
	player.controller.prevent_movement();
	m_world_shader->set_darken(m_map->darken_factor);
	m_world_shader->set_texture_size(m_map->real_dimensions / constants::f_scale_factor);

	m_airship.push_animation("main", {0, 4, 40, -1});
	m_airship.set_animation("main");
	m_intro_shot.start();
	m_location_text.set_bounds(sf::FloatRect({20.f, 480.f}, {600.f, 100.f}));

	for (auto i{0}; i < 4; ++i) {
		m_nighthawks.push_back(Nighthawk{svc});
		auto randx = random::random_range_float(600.f, 2000.f);
		auto randy = random::random_range_float(0.f, 400.f);
		m_nighthawks.back().steering.physics.position = {randx, randy};
		auto c = random::random_range(0, 2);
		auto f = random::random_range(0, 2);
		m_nighthawks.back().set_channel(0);
		m_nighthawks.back().set_frame(f);
		m_nighthawks.back().z = (3.f - static_cast<float>(c)) * 0.00002f;
	}
}

void Intro::tick_update(ServiceProvider& svc, capo::IEngine& engine) {

	if (svc.controller_map.digital_action_status(config::DigitalAction::platformer_toggle_pause).triggered || svc.controller_map.process_gamepad_disconnection()) {
		pause_window = std::make_unique<gui::PauseWindow>(svc, std::vector<std::string>{svc.data.gui_text["pause_menu"]["resume"].as_string(), svc.data.gui_text["pause_menu"]["settings"].as_string(),
																						svc.data.gui_text["pause_menu"]["controls"].as_string(), svc.data.gui_text["pause_menu"]["quit"].as_string()});
	}

	m_wait.update();
	if (m_wait.running()) { return; }

	for (auto& n : m_nighthawks) {
		n.steering.seek({-800.f, 400.f}, n.z);
		n.set_position(n.steering.physics.position);
	}

	m_location_text.update();
	if (m_location_text.is_writing()) { svc.soundboard.repeat_sound("console_speech"); }
	m_cloud_sea.update(svc);
	m_cloud.update(svc);
	m_airship.tick();
	m_airship.set_position(sf::Vector2f{0.f, 4.f * sin(m_intro_shot.get_normalized() * 10.f)});
	m_intro_shot.update();
	m_end_wait.update();
	m_attack_fadeout.update();

	player->controller.restrict_movement();

	if (!m_map) { return; }

	// cutscene logic
	if (m_intro_shot.get() == 1000) { m_location_text.start(); }
	if (m_intro_shot.is_almost_complete()) { m_map->transition.start(); }
	if (m_map->transition.is_black() && m_intro_shot.is_complete()) { set_flag(IntroFlags::established); }
	if (m_map->transition.is_black() && has_flag_set(IntroFlags::established) && !has_flag_set(IntroFlags::cutscene_started)) {
		svc.app_flags.set(AppFlags::in_game);
		m_map->cutscene_catalog.push_cutscene(svc, *m_map, *player, 1);
		m_map->transition.end();
		set_flag(IntroFlags::cutscene_started);
	}
	if (has_flag_set(IntroFlags::cutscene_started) && m_map->cutscene_catalog.cutscenes.empty()) {
		if (!has_flag_set(IntroFlags::cutscene_over)) { m_end_wait.start(); }
		set_flag(IntroFlags::cutscene_over);
	}
	if (has_flag_set(IntroFlags::cutscene_over)) {
		if (m_end_wait.is_almost_complete()) {
			m_console = std::make_unique<gui::Console>(svc, svc.text.basic, "intro", gui::OutputType::no_skip);
			svc.music_player.load(svc.finder, "none");
			svc.music_player.play_looped();
			set_flag(IntroFlags::console_message);
			m_attack_fadeout.start();
		}
		svc.ambience_player.set_balance(0.f);
		if (m_attack_fadeout.running()) { svc.ambience_player.set_volume(m_attack_fadeout.get_quadratic_normalized()); }
		if (!m_console && has_flag_set(IntroFlags::console_message)) { set_flag(IntroFlags::complete); }
	}

	if (has_flag_set(IntroFlags::complete) && m_map->cutscene_catalog.cutscenes.empty()) {
		svc.state_controller.actions.set(automa::Actions::intro_done);
		svc.state_controller.actions.set(automa::Actions::trigger);
		player->cooldowns.tutorial.start();
		svc.ticker.in_game_seconds_passed = {};
		svc.app_flags.set(automa::AppFlags::custom_map_start);
		player->set_sleeping();
		player->set_sleep_timer();
		player->set_direction(Direction{{1, 0}});
	}

	svc.a11y.set_action_ctx_bar_enabled(false);

	// set action set
	if (pause_window || m_console) {
		svc.controller_map.set_action_set(config::ActionSet::Menu);
		svc.controller_map.set_joystick_throttle({});
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

	// physical tick
	player->update(*m_map);
	player->start_tick();
	m_map->update(svc, m_console);

	m_map->debug_mode = debug_mode;

	player->end_tick();
	if (!m_console) { player->set_busy(false); }

	m_map->background->update(svc);
	hud.update(svc, *player);
}

void Intro::frame_update(ServiceProvider& svc) {}

void Intro::render(ServiceProvider& svc, sf::RenderWindow& win) {
	if (!m_map) { return; }
	if (m_world_shader) {
		m_map->render_background(svc, win, m_world_shader, player->get_camera_position());
		m_map->render(svc, win, m_world_shader, player->get_camera_position());
	}
	if (!has_flag_set(IntroFlags::established)) {
		m_cloud_sea.render(svc, win, {});
		win.draw(m_airship);
		// not sure about the nighthawks
		// for (auto& n : m_nighthawks) { win.draw(n); }
		m_cloud.render(svc, win, {});
		m_location_text.write_gradual_message(win);
	}
	m_map->transition.render(win);
	if (pause_window) { pause_window.value()->render(svc, win); }
	if (m_console) {
		m_console.value()->render(win);
		m_console.value()->write(win);
	}
}

void Intro::toggle_pause_menu(ServiceProvider& svc) { svc.ticker.paused() ? svc.ticker.unpause() : svc.ticker.pause(); }

} // namespace fornani::automa
