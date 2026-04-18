
#include <fornani/automa/SceneContext.hpp>
#include <fornani/entities/player/Player.hpp>
#include <fornani/gui/console/Console.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/story/cutscene/HaunchIntro.hpp>
#include <fornani/world/Map.hpp>

namespace fornani {

HaunchIntro::HaunchIntro(automa::ServiceProvider& svc)
	: Cutscene(svc, 900, "haunch_intro"), m_intro{200}, m_army_truck_body{svc, "army_truck_body", {203, 132}}, m_army_truck_undercarriage{svc, "army_truck_undercarriage", {203, 132}},
	  m_truck_path{svc.finder, std::filesystem::path{"/data/vfx/scenery_paths.json"}, "army_truck", 1000, util::InterpolationType::quadratic}, m_hulmet_spawn_delay{140} {
	m_intro.start();
	svc.music_player.load(svc.finder, "haunchs_theme");
	svc.input_system.flush_inputs();
	svc.state_flags.set(automa::StateFlags::cutscene);
	m_truck_path.set_section("arrive");
	m_army_truck_undercarriage.push_and_set_animation("moving", {0, 2, 32, -1});
	m_army_truck_undercarriage.push_animation("stopped", {0, 1, 32, -1});
}

void HaunchIntro::update(automa::ServiceProvider& svc, SceneContext& context, world::Map& map, player::Player& player) {
	if (m_flags.consume(HaunchIntroFlags::done)) {
		player.controller.unrestrict();
		svc.state_flags.reset(automa::StateFlags::hide_hud);
		svc.state_flags.reset(automa::StateFlags::no_menu);
		svc.state_flags.reset(automa::StateFlags::cutscene);
		svc.camera_controller.set_owner(graphics::CameraOwner::player);
		svc.music_player.stop();
		svc.music_player.load("scuffle");
		svc.music_player.play_looped();
		player.set_flag(player::PlayerFlags::cutscene, false);
		svc.camera_controller.constrain();
		m_flags.set(HaunchIntroFlags::over);
		m_truck_path.set_section("depart");
		m_truck_path.set_interpolation_type(util::InterpolationType::linear);
		m_army_truck_undercarriage.set_animation("moving");
		return;
	}

	if (m_flags.test(HaunchIntroFlags::over)) {
		m_truck_path.update();
		if (m_truck_path.completed_step(2)) { flags.set(CutsceneFlags::delete_me); }
		return;
	}

	svc.state_flags.set(automa::StateFlags::hide_hud);
	svc.state_flags.set(automa::StateFlags::no_menu);
	svc.state_flags.set(automa::StateFlags::cutscene);
	cooldowns.beginning.update();
	cooldowns.pause.update();
	cooldowns.long_pause.update();
	cooldowns.end.update();
	m_hulmet_spawn_delay.update();

	m_truck_path.update();
	m_army_truck_undercarriage.tick();

	player.controller.restrict_movement();
	player.stall_idle_timer();
	player.set_flag(player::PlayerFlags::show_weapon, false);

	auto npcs = map.get_entities<NPC>();
	auto hit = std::ranges::find_if(npcs, [](auto& n) { return n->get_specifier() == 39; });
	auto& haunch = *hit;

	if (m_intro.just_started()) {
		player.controller.prevent_movement();
		player.get_collider().physics.zero_x();
		player.set_flag(player::PlayerFlags::cutscene);
		player.set_idle();
		haunch->set_flag(NPCFlags::cutscene);
	}
	if (m_intro.is_almost_complete()) { cooldowns.beginning.start(); }
	m_intro.update();
	if (m_intro.running()) { return; }

	if (context.console) { context.console.value()->set_no_exit(true); }

	auto total_suites{0};
	for (auto& npc : npcs) { total_suites += npc->get_number_of_suites(); }
	total_conversations = std::max(total_conversations, total_suites);
	if (cooldowns.end.is_almost_complete()) {
		flags.set(CutsceneFlags::complete);
		return;
	}
	if (npcs.empty()) { return; }

	if (cooldowns.end.running()) { haunch->disengage(); }
	if (cooldowns.beginning.is_almost_complete()) {}
	if (context.console) { haunch->disengage(); }

	svc.camera_controller.set_owner(graphics::CameraOwner::system);
	svc.camera_controller.free();
	svc.camera_controller.set_position(haunch->Mobile::get_global_center());

	switch (progress) {
	case 0:
		if (m_truck_path.completed_step(1)) {
			++progress;
			map.spawn_enemy(28, m_truck_path.get_position() + sf::Vector2f{180.f, 64.f}, 0, false, false);
			m_army_truck_undercarriage.set_animation("stopped");
			svc.soundboard.play_sound("train_steam");
		}
		break;
	case 1:
		svc.music_player.play_looped();
		haunch->force_engage();
		m_hulmet_spawn_delay.start();
		++progress;
		break;
	case 2:
		if (m_hulmet_spawn_delay.is_almost_complete()) {
			map.spawn_enemy(13, m_truck_path.get_position() + sf::Vector2f{140.f, 80.f}, 0, false, false);
			map.spawn_enemy(13, m_truck_path.get_position() + sf::Vector2f{240.f, 80.f}, 0, false, false);
			++progress;
		}
		break;
	case 3: break;
	case 4: break;
	case 10:
		if (!context.console.has_value()) {
			m_flags.set(HaunchIntroFlags::done);
			svc.music_player.load(svc.finder, "scuffle");
			svc.music_player.play_looped();
			++progress;
		}
		break;
	case 11: break;
	default: break;
	}
}

void HaunchIntro::render(sf::RenderWindow& win, sf::Vector2f cam) {
	m_army_truck_body.set_position(m_truck_path.get_position() - cam);
	m_army_truck_undercarriage.set_position(m_truck_path.get_position() - cam);
	win.draw(m_army_truck_undercarriage);
	win.draw(m_army_truck_body);
}

} // namespace fornani
