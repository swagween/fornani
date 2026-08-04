
#include <fornani/automa/SceneContext.hpp>
#include <fornani/entities/player/Player.hpp>
#include <fornani/gui/console/Console.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/story/cutscene/RetrieveLynx.hpp>
#include <fornani/world/Map.hpp>

namespace fornani {

RetrieveLynx::RetrieveLynx(automa::ServiceProvider& svc, world::Map& map, player::Player& player)
	: Cutscene(svc, 607, "retrieve_lynx"), m_cloud_sea{svc, "stormcloud_sea", {1024, 512}}, m_greatwing{svc, "scenery_greatwing_airship", {480, 256}}, m_nighthawk{svc, "scenery_very_distant_nighthawk"}, m_services{&svc}, m_intro{1800},
	  m_airship_movement{3200}, m_location_text{svc, svc.data.gui_text["locations"]["greatwing"].as_string_view()}, m_pre_intro{300} {
	svc.events.load_room_event.dispatch(svc, 607);
	svc.world_clock.set_time(8, 30);
	m_intro.start();
	m_pre_intro.start();
	m_greatwing.push_and_set_animation("basic", {0, 3, 20, -1});
	m_airship_movement.start();
	m_location_text.set_bounds(sf::FloatRect({20.f, 480.f}, {600.f, 100.f}));
	m_nighthawk_steering.physics.position = svc.window->f_screen_dimensions() + sf::Vector2f{-100.f, -300.f};
	svc.music_player.stop();
	svc.music_player.load(svc.finder, "brown");
	svc.music_player.play_looped();

	auto npcs = map.get_entities<NPC>();
	auto mit = std::ranges::find_if(npcs, [](auto& n) { return n->get_specifier() == 7; });
	if (mit == npcs.end()) {
		NANI_LOG_ERROR(p_logger, "Missing NPC from cutscene!");
		return;
	}
	auto& gus = *mit;

	gus->give_prop(svc, map, "unconscious_lynx", {32, 32});
	gus->get_prop().push_and_set_animation("limp", {0, 1, 8, -1});
	gus->get_prop().push_animation("fall", {1, 7, 38, 0, true});
	gus->give_vehicle(svc, map, "nighthawk");
	gus->set_flag(NPCFlags::in_vehicle);
	m_flags.set(RetrieveLynxFlags::holding_lynx);
}

void RetrieveLynx::update(automa::ServiceProvider& svc, SceneContext& context, world::Map& map, player::Player& player) {
	if (!m_flags.test(RetrieveLynxFlags::pre_intro) && !m_flags.test(RetrieveLynxFlags::hang)) {
		context.transition.hang();
		m_flags.set(RetrieveLynxFlags::hang);
	}
	if (context.transition.is_black() && !m_flags.test(RetrieveLynxFlags::pre_intro) && m_intro.started() && !m_flags.test(RetrieveLynxFlags::intro_done) && m_pre_intro.is_almost_complete()) {
		context.console = std::make_unique<gui::Console>(svc, svc.text.basic, "far_away", gui::OutputType::no_skip);
		m_flags.set(RetrieveLynxFlags::pre_intro);
	}

	if (complete() && context.transition.is(graphics::TransitionState::inactive)) {
		context.transition.start();
		return;
	} else if (complete() && context.transition.is(graphics::TransitionState::black)) {
		svc.state_controller.switch_rooms(1111, metadata.target_state_on_end, context.transition);
		svc.state_controller.player_position = sf::Vector2f{3.f, 8.f} * constants::f_cell_size;
		svc.state_controller.actions.set(automa::Actions::custom_player_position);
		player.set_sleeping(true);
		player.set_sleep_timer(512);
		svc.app_flags.set(automa::AppFlags::custom_map_start);
		svc.world_clock.set_time(17, 30);
		svc.events.transition_event.dispatch();
		svc.quest_table.set_quest_progression("retrieve_lynx", 1);
		Cutscene::end(svc, player);
		return;
	}

	// always has to be called
	for (auto& e : m_smoke_effects) { e.update(); }
	if (m_intro.just_started()) {
		svc.music_player.stop();
		svc.music_player.load(svc.finder, "wind");
		svc.music_player.play_looped();
	}
	if (metadata.no_player) {
		player.set_position({3200.f, -64.f});
		player.controller.restrict_movement();
		player.get_collider().physics.zero_y();
	}
	svc.state_flags.set(automa::StateFlags::hide_hud);
	svc.state_flags.set(automa::StateFlags::no_menu);
	svc.state_flags.set(automa::StateFlags::cutscene);
	player.set_flag(player::PlayerFlags::cutscene);
	auto npcs = map.get_entities<NPC>();
	auto nit = std::ranges::find_if(npcs, [](auto& n) { return n->get_specifier() == 22; });
	auto mit = std::ranges::find_if(npcs, [](auto& n) { return n->get_specifier() == 7; });
	if (nit == npcs.end() || mit == npcs.end()) {
		NANI_LOG_ERROR(p_logger, "Missing NPC from cutscene!");
		return;
	}
	auto& nimbus = *nit;
	auto& gus = *mit;
	nimbus->set_flag(NPCFlags::custom_camera);
	gus->set_flag(NPCFlags::custom_camera);
	nimbus->set_flag(NPCFlags::cutscene);
	gus->set_flag(NPCFlags::cutscene);
	if (context.console.has_value()) { nimbus->disengage(); }
	if (context.console.has_value()) { gus->disengage(); }
	auto camera_focus = (nimbus->get_collider().get_center() + gus->get_collider().get_center()) * 0.5f + sf::Vector2f{0.f, 49.f};
	if (progress >= 17) { camera_focus = nimbus->get_collider().get_center() + sf::Vector2f{0.f, 49.f}; }
	if (progress > 20) { camera_focus = nimbus->get_collider().get_center() + sf::Vector2f{200.f, 60.f}; }
	svc.camera_controller.set_owner(graphics::CameraOwner::system);
	svc.camera_controller.set_position(camera_focus);

	// intro logic
	m_pre_intro.update();
	if (m_pre_intro.running()) {
		context.transition.hang();
		return;
	}
	if (m_flags.test(RetrieveLynxFlags::intro_done)) { m_intro.update(); }
	if (context.console && m_flags.test(RetrieveLynxFlags::pre_intro)) {
		context.transition.hang();
		return;
	}

	m_greatwing.tick();
	m_cloud_sea.update(svc);
	m_greatwing.set_position(sf::Vector2f{-40.f, 2.f * sin(m_airship_movement.get_normalized() * 20.f) + -20.f});
	m_airship_movement.update();
	m_location_text.update();
	if (m_location_text.is_writing()) { svc.soundboard.repeat_sound("console_speech"); }
	if (m_intro.get() == 1200) { m_location_text.start(); }
	m_nighthawk_steering.thrust_seek(sf::Vector2f{40.f, 300.f}, {0.031f, .118f, .991f, 260.f});
	m_nighthawk_steering.steering.smooth_random_walk(m_nighthawk_steering.physics, 0.007f, 48.f);
	m_nighthawk.set_position(m_nighthawk_steering.physics.position);
	if (svc.ticker.every_x_ticks(30)) { m_smoke_effects.push_back(entity::Effect(svc, "tiny_smoke", m_nighthawk_steering.physics.position + sf::Vector2f{16.f, 0.f})); }
	if (m_intro.is_almost_complete()) {
		cooldowns.beginning.start(512);
		context.transition.start();
		m_flags.set(RetrieveLynxFlags::main_scene);
		svc.music_player.stop();
		svc.music_player.load(svc.finder, "skycorps");
		svc.music_player.play_looped();
		gus->request(NPCAnimationState::special_1);
	}
	if (context.transition.is_black() && m_flags.test(RetrieveLynxFlags::main_scene)) {
		m_flags.reset(RetrieveLynxFlags::main_scene);
		context.transition.end();
		m_gus_steering.physics.position = sf::Vector2f{28.f, 16.f} * constants::f_cell_size;
		if (!m_flags.test(RetrieveLynxFlags::arrived)) { gus->set_position(m_gus_steering.physics.position); }
	}
	if (context.transition.is(graphics::TransitionState::inactive)) { cooldowns.beginning.update(); }
	cooldowns.pause.update();
	cooldowns.long_pause.update();
	cooldowns.end.update();

	if (!context.console && m_flags.test(RetrieveLynxFlags::pre_intro)) {
		m_flags.reset(RetrieveLynxFlags::pre_intro);
		m_flags.set(RetrieveLynxFlags::intro_done);
	}
	if (m_intro.running()) { return; }

	if (context.console) { context.console.value()->set_no_exit(true); }

	auto total_suites{0};

	for (auto n : npcs) { total_suites += n->get_number_of_suites(); }
	total_conversations = std::max(total_conversations, total_suites);
	if (cooldowns.end.get() == 1) {
		flags.set(CutsceneFlags::complete);
		return;
	}
	if (npcs.size() < 2) { return; }

	auto target = sf::Vector2f{18.f, 21.f} * constants::f_cell_size;
	m_gus_steering.thrust_seek(target, {0.08f, .118f, .991f, 260.f});
	if (!m_flags.test(RetrieveLynxFlags::arrived)) { gus->set_position(m_gus_steering.physics.position); }
	if (gus->has_vehicle()) {
		if (gus->get_vehicle().is_close_to_point(target, 32.f) && cooldowns.beginning.halfway()) {
			gus->set_flag(NPCFlags::airborne, false);
			gus->set_flag(NPCFlags::in_vehicle, false);
			gus->get_collider().set_attribute(shape::ColliderAttributes::no_map_collision, false);
			gus->get_collider().set_attribute(shape::ColliderAttributes::no_collision, false);
			gus->get_collider().set_flag(shape::ColliderFlags::gravity);
			gus->get_collider().set_flag(shape::ColliderFlags::simple, false);
			gus->get_collider().physics.velocity.y = -2.f;
			gus->get_collider().physics.gravity = 0.1f;
			m_flags.set(RetrieveLynxFlags::arrived);
		}
	}
	auto lynx_offset = progress < 9 || progress > 16 ? sf::Vector2f{-56.f, -65.f} : sf::Vector2f{-56.f, 0.f};
	m_lynx_target = gus->get_collider().get_center() + lynx_offset;
	gus->set_prop_socket(m_lynx_target);
	auto channel = m_flags.test(RetrieveLynxFlags::holding_lynx) ? 1 : 0;
	gus->set_channel(channel);

	if (cooldowns.beginning.running()) {
		nimbus->walk();
		if (cooldowns.beginning.is_almost_complete()) { nimbus->request(NPCAnimationState::idle); }
		return;
	}

	// dialog
	switch (progress) {
	case 0:
		if (!context.console && m_flags.test(RetrieveLynxFlags::gus_landed) && cooldowns.pause.is_complete()) {
			nimbus->flush_and_push(60);
			nimbus->force_engage();
			nimbus->request(NPCAnimationState::special_1);
			cooldowns.long_pause.start(400);
			++progress;
			return;
		}
		if (gus->get_collider().grounded() && !cooldowns.pause.running()) {
			m_flags.set(RetrieveLynxFlags::gus_landed);
			svc.soundboard.play_sound("heavy_land");
			svc.camera_controller.shake(10, 0.3f, 200, 20);
			cooldowns.pause.start();
			gus->set_flag(NPCFlags::background, false);
		}
		break;
	case 1:
		if (cooldowns.long_pause.running()) {
			gus->walk();
			break;
		}
		gus->request(NPCAnimationState::idle);
		if (!context.console) {
			gus->flush_and_push(60);
			gus->force_engage();
			++progress;
			return;
		}
		break;
	case 2:
		if (!context.console) {
			cooldowns.pause.start(128);
			++progress;
		}
		break;
	case 3:
		if (cooldowns.pause.is_almost_complete()) { ++progress; }

		break;
	case 4:
		if (!context.console) {
			gus->flush_and_push(61);
			gus->force_engage();
			cooldowns.pause.start();
			++progress;
			return;
		}
		break;
	case 5:
		if (!context.console) {
			nimbus->flush_and_push(61);
			nimbus->force_engage();
			cooldowns.pause.start();
			++progress;
			return;
		}
		break;
	case 6: break;
	case 8:
		if (!context.console) {
			if (gus->has_prop()) {
				gus->get_prop().set_animation("fall");
				gus->drop_prop();
				gus->request(NPCAnimationState::idle);
				m_flags.reset(RetrieveLynxFlags::holding_lynx);
			}
			cooldowns.pause.start(450);
			++progress;
		}
		break;
	case 9:
		nimbus->walk();
		if (cooldowns.pause.is_almost_complete()) {
			nimbus->request(NPCAnimationState::inspect);
			nimbus->flush_and_push(62);
			nimbus->force_engage();
			++progress;
		}
		break;
	case 10:
		if (!context.console) {
			cooldowns.pause.start(300);
			++progress;
		}
		break;
	case 11:
		if (cooldowns.pause.is_almost_complete()) {
			nimbus->flush_and_push(63);
			nimbus->force_engage();
			++progress;
		}
		break;
	case 12:
		if (!context.console) {
			gus->flush_and_push(62);
			gus->force_engage();
			++progress;
		}
		break;
	case 13:
		if (!context.console) {
			nimbus->flush_and_push(64);
			nimbus->force_engage();
			++progress;
		}
		break;
	case 14:
		if (!context.console) {
			gus->flush_and_push(63);
			gus->force_engage();
			++progress;
		}
		break;
	case 15:
		if (!context.console) {
			nimbus->flush_and_push(65);
			nimbus->force_engage();
			++progress;
		}
		break;
	case 16:
		if (!context.console) {
			if (gus->has_prop()) {
				gus->get_prop().set_animation("limp");
				gus->pick_up_prop();
				gus->request(NPCAnimationState::special_3);
				m_flags.set(RetrieveLynxFlags::holding_lynx);
			}
			cooldowns.long_pause.start();
			++progress;
		}
		break;
	case 17:
		nimbus->walk();
		gus->walk();
		if (cooldowns.long_pause.is_almost_complete()) {
			nimbus->request(NPCAnimationState::inspect);
			cooldowns.pause.start(1000);
			++progress;
		}
		break;
	case 18:
		if (cooldowns.pause.is_almost_complete()) {
			nimbus->flush_and_push(66);
			nimbus->force_engage();
			++progress;
		}
		break;
	case 19:
		if (!context.console) {
			cooldowns.pause.start(300);
			++progress;
		}
		break;
	case 20:
		if (cooldowns.pause.is_almost_complete()) {
			nimbus->request(NPCAnimationState::special_1);
			nimbus->flush_and_push(67);
			nimbus->force_engage();
			++progress;
		}
		break;
	case 30:
		if (!context.console && !cooldowns.end.running()) {
			cooldowns.end.start();
			nimbus->pop_conversation();
			return;
		}
		if (!context.console && cooldowns.end.is_almost_complete()) { return; }
		break;
	default: break;
	}

	if (progress >= 17) { gus->use_portal(map); }

	if (gus->has_prop()) {
		if (gus->get_prop().get_animation().get_frame() == 4 && gus->get_prop().get_animation().keyframe_started()) { svc.soundboard.play_sound("basic_land"); }
	}
}

void RetrieveLynx::render(sf::RenderWindow& win, sf::Vector2f cam) {}

void RetrieveLynx::render_on_top(sf::RenderWindow& win, sf::Vector2f cam) {
	if (m_intro.running() || m_flags.test(RetrieveLynxFlags::main_scene)) {
		m_cloud_sea.render(*m_services, win, {});
		win.draw(m_nighthawk);
		for (auto& e : m_smoke_effects) { e.render(win, cam); }
		win.draw(m_greatwing);
		m_location_text.write_gradual_message(win);
	}
}

} // namespace fornani
