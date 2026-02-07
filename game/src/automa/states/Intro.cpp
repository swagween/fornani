
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
	svc.state_flags.set(automa::StateFlags::cutscene);

	player.set_camera_bounds(m_map->real_dimensions);
	player.force_camera_center();

	player.set_position(sf::Vector2f{14.5f, 10.f} * constants::f_cell_size);

	p_world_shader = LightShader(svc.finder);
	svc.a11y.set_action_ctx_bar_enabled(false);
	player.controller.prevent_movement();
	p_world_shader->set_darken(m_map->darken_factor);
	p_world_shader->set_texture_size(m_map->real_dimensions / constants::f_scale_factor);

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

	set_flag(GameplayStateFlags::early_tick_return, false);
	GameplayState::tick_update(svc, engine);
	if (has_flag_set(GameplayStateFlags::early_tick_return)) { return; }

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
	if (m_map->transition.is_black() && m_intro_shot.is_complete()) { m_flags.set(IntroFlags::established); }
	if (m_map->transition.is_black() && m_flags.test(IntroFlags::established) && !m_flags.test(IntroFlags::cutscene_started)) {
		svc.app_flags.set(AppFlags::in_game);
		m_map->cutscene_catalog.push_cutscene(svc, *m_map, *player, 1);
		m_map->transition.end();
		m_flags.set(IntroFlags::cutscene_started);
	}
	if (m_flags.test(IntroFlags::cutscene_started) && m_map->cutscene_catalog.cutscenes.empty()) {
		if (!m_flags.test(IntroFlags::cutscene_over)) { m_end_wait.start(); }
		m_flags.set(IntroFlags::cutscene_over);
	}
	if (m_flags.test(IntroFlags::cutscene_over)) {
		if (m_end_wait.is_almost_complete()) {
			m_console = std::make_unique<gui::Console>(svc, svc.text.basic, "intro", gui::OutputType::no_skip);
			svc.music_player.load(svc.finder, "none");
			svc.music_player.play_looped();
			m_flags.set(IntroFlags::console_message);
			m_attack_fadeout.start();
		}
		svc.ambience_player.set_balance(0.f);
		if (m_attack_fadeout.running()) { svc.ambience_player.set_volume(m_attack_fadeout.get_quadratic_normalized()); }
		if (!m_console && m_flags.test(IntroFlags::console_message)) { m_flags.set(IntroFlags::complete); }
	}

	if (m_flags.test(IntroFlags::complete) && m_map->cutscene_catalog.cutscenes.empty()) {
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
	if (p_world_shader) {
		m_map->render_background(svc, win, p_world_shader, player->get_camera_position());
		m_map->render(svc, win, p_world_shader, player->get_camera_position());
	}
	if (!m_flags.test(IntroFlags::established)) {
		m_cloud_sea.render(svc, win, {});
		win.draw(m_airship);
		// not sure about the nighthawks
		// for (auto& n : m_nighthawks) { win.draw(n); }
		m_cloud.render(svc, win, {});
		m_location_text.write_gradual_message(win);
	}
	GameplayState::render(svc, win);
}

} // namespace fornani::automa
