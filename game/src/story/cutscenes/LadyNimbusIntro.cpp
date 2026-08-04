
#include <fornani/automa/SceneContext.hpp>
#include <fornani/entities/player/Player.hpp>
#include <fornani/gui/console/Console.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/story/cutscene/LadyNimbusIntro.hpp>
#include <fornani/world/Map.hpp>

namespace fornani {

LadyNimbusIntro::LadyNimbusIntro(automa::ServiceProvider& svc)
	: Cutscene(svc, 601, "lady_nimbus_intro"), m_cloud_sea{svc, "duskcloud_sea", {1024, 512}}, m_greatwing{svc, "scenery_greatwing_airship", {480, 256}}, m_intro{1200}, m_airship_movement{3200}, m_pre_intro{300},
	  m_location_text{svc, svc.data.gui_text["locations"]["greatwing"].as_string_view()}, m_services{&svc} {
	cooldowns.beginning.start();
	svc.world_clock.set_time(9, 30);
	m_greatwing.push_and_set_animation("basic", {0, 3, 80, -1});
	m_airship_movement.start();
	m_location_text.set_bounds(sf::FloatRect({20.f, 480.f}, {600.f, 100.f}));
	m_intro.start();
	m_pre_intro.start();
	svc.camera_controller.set_owner(graphics::CameraOwner::system);
}

void LadyNimbusIntro::update(automa::ServiceProvider& svc, SceneContext& context, world::Map& map, player::Player& player) {

	if (!m_flags.test(LadyNimbusIntroFlags::pre_intro) && !m_flags.test(LadyNimbusIntroFlags::hang)) {
		context.transition.hang();
		m_flags.set(LadyNimbusIntroFlags::hang);
	}
	if (context.transition.is_black() && !m_flags.test(LadyNimbusIntroFlags::pre_intro) && m_intro.started() && !m_flags.test(LadyNimbusIntroFlags::intro_done) && m_pre_intro.is_almost_complete()) {
		context.console = std::make_unique<gui::Console>(svc, svc.text.basic, "far_away", gui::OutputType::no_skip);
		m_flags.set(LadyNimbusIntroFlags::pre_intro);
	}

	if (complete() && context.transition.is(graphics::TransitionState::inactive)) {
		context.transition.start();
		return;
	} else if (complete() && context.transition.is(graphics::TransitionState::black)) {
		svc.state_controller.switch_rooms(199, metadata.target_state_on_end, context.transition);
		svc.state_controller.player_position = sf::Vector2f{13, 60} * constants::f_cell_size;
		svc.state_controller.actions.set(automa::Actions::custom_player_position);
		svc.state_flags.reset(automa::StateFlags::no_menu);
		svc.state_flags.reset(automa::StateFlags::cutscene);
		player.set_sleeping(true);
		player.set_sleep_timer(1024);
		svc.app_flags.set(automa::AppFlags::custom_map_start);
		svc.world_clock.set_time(9, 30);
		svc.events.transition_event.dispatch();
		flags.set(CutsceneFlags::delete_me);
		svc.camera_controller.set_owner(graphics::CameraOwner::player);
		return;
	}

	// always has to be called
	svc.camera_controller.constrain();
	if (metadata.no_player) {
		player.set_position({0.f, -64.f});
		player.controller.restrict_movement();
		player.get_collider().physics.zero_y();
	}
	svc.state_flags.set(automa::StateFlags::hide_hud);
	svc.state_flags.set(automa::StateFlags::no_menu);
	svc.state_flags.set(automa::StateFlags::cutscene);

	// intro logic
	m_pre_intro.update();
	if (m_pre_intro.running()) {
		context.transition.hang();
		return;
	}
	if (m_intro.just_started()) {
		svc.music_player.stop();
		svc.music_player.load(svc.finder, "wind");
		svc.music_player.play_looped();
	}
	if (m_flags.test(LadyNimbusIntroFlags::intro_done)) { m_intro.update(); }
	if (context.console && m_flags.test(LadyNimbusIntroFlags::pre_intro)) {
		context.transition.hang();
		return;
	}

	m_greatwing.tick();
	m_cloud_sea.update(svc);
	m_greatwing.set_position(sf::Vector2f{-40.f, 2.f * sin(m_airship_movement.get_normalized() * 20.f) + -20.f});
	m_airship_movement.update();
	m_location_text.update();
	if (m_location_text.is_writing()) { svc.soundboard.repeat_sound("console_speech"); }
	if (m_intro.get() == 600) { m_location_text.start(); }
	if (m_intro.is_almost_complete()) {
		cooldowns.beginning.start(380);
		context.transition.start();
		m_flags.set(LadyNimbusIntroFlags::main_scene);
		svc.music_player.stop();
		svc.music_player.load(svc.finder, "skycorps");
		svc.music_player.play_looped();
	}
	if (context.transition.is_black() && m_flags.test(LadyNimbusIntroFlags::main_scene)) {
		m_flags.reset(LadyNimbusIntroFlags::main_scene);
		context.transition.end();
	}
	if (context.transition.is(graphics::TransitionState::inactive)) { cooldowns.beginning.update(); }
	cooldowns.pause.update();
	cooldowns.long_pause.update();
	cooldowns.end.update();

	if (!context.console && m_flags.test(LadyNimbusIntroFlags::pre_intro)) {
		m_flags.reset(LadyNimbusIntroFlags::pre_intro);
		m_flags.set(LadyNimbusIntroFlags::intro_done);
	}
	if (m_intro.running()) { return; }

	if (context.console) { context.console.value()->set_no_exit(true); }

	auto total_suites{0};

	auto npcs = map.get_entities<NPC>();
	auto nit = std::ranges::find_if(npcs, [](auto& n) { return n->get_specifier() == 22; });
	auto hit = std::ranges::find_if(npcs, [](auto& n) { return n->get_specifier() == 7; });

	if (nit == npcs.end() || hit == npcs.end()) {
		NANI_LOG_ERROR(p_logger, "Missing NPC from cutscene!");
		return;
	}
	auto& nimbus = *nit;
	auto& hologus = *hit;
	if (context.console.has_value()) { nimbus->disengage(); }
	if (context.console.has_value()) { hologus->disengage(); }
	nimbus->set_busy();
	nimbus->set_flag(NPCFlags::custom_camera);
	hologus->set_flag(NPCFlags::custom_camera);
	auto camera_focus = nimbus->get_collider().get_center() + sf::Vector2f{0.f, 0.f};
	svc.camera_controller.set_owner(graphics::CameraOwner::system);
	svc.camera_controller.set_position(camera_focus);

	for (auto n : npcs) { total_suites += n->get_number_of_suites(); }
	total_conversations = std::max(total_conversations, total_suites);
	if (cooldowns.end.get() == 1) {
		flags.set(CutsceneFlags::complete);
		return;
	}
	if (npcs.size() < 2) { return; }
	if (cooldowns.beginning.running()) { return; }

	// dialog
	switch (progress) {
	case 0:
		if (!context.console) {
			nimbus->force_engage();
			++progress;
			return;
		}
		break;
	case 1:
		if (cooldowns.long_pause.get() == 500) { svc.soundboard.flags.transmission.set(audio::Transmission::statics); }
		if (!context.console && !cooldowns.long_pause.running()) {
			cooldowns.long_pause.start();
			nimbus->pop_conversation();
			return;
		}
		if (!context.console && cooldowns.long_pause.get() == 1) {
			hologus->force_engage();
			++progress;
			return;
		}
		break;
	case 2:
		if (context.console) { context.console.value()->set_hologram(); }
		if (!context.console && !cooldowns.pause.running()) {
			cooldowns.pause.start();
			hologus->pop_conversation();
			return;
		}
		if (!context.console && cooldowns.pause.get() == 1) {
			nimbus->force_engage();
			++progress;
			return;
		}
		break;
	case 3:
		if (!context.console && !cooldowns.pause.running()) {
			cooldowns.pause.start();
			nimbus->pop_conversation();
			return;
		}
		if (!context.console && cooldowns.pause.get() == 1) {
			hologus->force_engage();
			++progress;
			return;
		}
		break;
	case 4:
		if (context.console) { context.console.value()->set_hologram(); }
		if (!context.console && !cooldowns.pause.running()) {
			cooldowns.pause.start();
			hologus->pop_conversation();
			return;
		}
		if (!context.console && cooldowns.pause.get() == 1) {
			nimbus->force_engage();
			++progress;
			return;
		}
		break;
	case 5:
		if (!context.console && !cooldowns.pause.running()) {
			cooldowns.pause.start();
			nimbus->pop_conversation();
			return;
		}
		if (!context.console && cooldowns.pause.get() == 1) {
			hologus->force_engage();
			++progress;
			return;
		}
		break;
	case 6:
		if (context.console) { context.console.value()->set_hologram(); }
		if (!context.console && !cooldowns.pause.running()) {
			cooldowns.pause.start();
			hologus->pop_conversation();
			return;
		}
		if (!context.console && cooldowns.pause.get() == 1) {
			nimbus->force_engage();
			++progress;
			return;
		}
		break;
	case 7:
		if (!context.console && !cooldowns.pause.running()) {
			cooldowns.pause.start();
			nimbus->pop_conversation();
			return;
		}
		if (!context.console && cooldowns.pause.get() == 1) {
			hologus->force_engage();
			++progress;
			return;
		}
		break;
	case 8:
		if (context.console) { context.console.value()->set_hologram(); }
		if (!context.console && !cooldowns.pause.running()) {
			cooldowns.pause.start();
			hologus->pop_conversation();
			return;
		}
		if (!context.console && cooldowns.pause.get() == 1) {
			nimbus->force_engage();
			++progress;
			return;
		}
		break;
	case 9:
		if (!context.console && !cooldowns.end.running()) {
			cooldowns.end.start();
			nimbus->pop_conversation();
			return;
		}
		if (!context.console && cooldowns.end.get() == 1) { return; }
		break;
	default: break;
	}
}

void LadyNimbusIntro::render_on_top(sf::RenderWindow& win, sf::Vector2f cam) {
	if (m_intro.running() || m_flags.test(LadyNimbusIntroFlags::main_scene)) {
		m_cloud_sea.render(*m_services, win, {});
		win.draw(m_greatwing);
		m_location_text.write_gradual_message(win);
	}
}

} // namespace fornani
