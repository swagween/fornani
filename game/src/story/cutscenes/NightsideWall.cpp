
#include <fornani/entities/player/Player.hpp>
#include <fornani/gui/console/Console.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/story/cutscene/NightsideWall.hpp>
#include <fornani/world/Map.hpp>

namespace fornani {

NightsideWall::NightsideWall(automa::ServiceProvider& svc) : Cutscene(svc, 903, "nightside_wall"), m_intro{1200} {
	m_intro.start();
	svc.music_player.stop();
	svc.music_player.load(svc.finder, "glitchified");
	NANI_LOG_DEBUG(p_logger, "Pushed cutscene");
	debug = true;
	if (svc.quest_table.get_quest_progression("nightside_wall") == 2) {
		progress = 7;
		m_intro.start(480);
	} else {
		svc.soundboard.play_sound("phone_ring_outgoing");
	}
}

void NightsideWall::update(automa::ServiceProvider& svc, std::optional<std::unique_ptr<gui::Console>>& console, world::Map& map, player::Player& player) {
	if (complete()) {
		player.controller.unrestrict();
		svc.state_flags.reset(automa::StateFlags::hide_hud);
		svc.state_flags.reset(automa::StateFlags::no_menu);
		svc.state_flags.reset(automa::StateFlags::cutscene);
		svc.camera_controller.set_owner(graphics::CameraOwner::player);
		// svc.quest_table.progress_quest("defeat_miaag", 1, 50901);
		flags.set(CutsceneFlags::delete_me);
		svc.music_player.stop();
		svc.music_player.load(svc.finder, "wind");
		svc.music_player.play_looped();
		return;
	}

	svc.state_flags.set(automa::StateFlags::hide_hud);
	svc.state_flags.set(automa::StateFlags::no_menu);
	svc.state_flags.set(automa::StateFlags::cutscene);
	cooldowns.beginning.update();
	cooldowns.pause.update();
	cooldowns.long_pause.update();
	cooldowns.end.update();

	if (!(progress == 7 && console.has_value())) { m_intro.update(); }
	if (m_intro.is_almost_complete()) { cooldowns.beginning.start(); }
	if (m_intro.running()) {
		if (m_intro.get() == 400) { svc.soundboard.play_sound("phone_dial"); }
		return;
	}

	player.controller.restrict_movement();

	if (console) { console.value()->set_no_exit(true); }

	auto npcs = map.get_entities<NPC>();
	auto bit = std::ranges::find_if(npcs, [](auto& n) { return n->get_specifier() == 34; });
	auto& bryn = *bit;

	auto total_suites{0};
	for (auto& npc : npcs) { total_suites += npc->get_number_of_suites(); }
	total_conversations = std::max(total_conversations, total_suites);
	if (cooldowns.end.is_almost_complete()) {
		flags.set(CutsceneFlags::complete);
		return;
	}
	if (npcs.empty()) { return; }

	if (cooldowns.end.running()) { bryn->disengage(); }
	if (cooldowns.beginning.is_almost_complete()) {
		map.reveal_npc("aviator_bryn");
		bryn->set_invisible();
		svc.music_player.play_looped();
		if (!console.has_value()) { bryn->force_engage(); }
	}
	if (console) { bryn->disengage(); }

	auto camera_focus = player.get_camera_focus_point();
	if (progress > 0) { camera_focus = sf::Vector2f{31.f, 77.f} * constants::f_cell_size; }

	switch (progress) {
	case 0: break;
	case 1: break;
	case 2:
		svc.data.switch_destructible_state(90302);
		++progress;
		break;
	case 3: break;
	case 4: camera_focus = sf::Vector2f{40.f, 25.f} * constants::f_cell_size; break;
	case 5:
		camera_focus = player.get_camera_focus_point();
		if (!console.has_value()) {
			cooldowns.end.start();
			++progress;
		}
		break;
	case 6: camera_focus = player.get_camera_focus_point(); break;
	case 7: camera_focus = player.get_camera_focus_point(); break;
	case 8: camera_focus = sf::Vector2f{27.f, 80.f} * constants::f_cell_size; break;
	case 9:
		camera_focus = sf::Vector2f{27.f, 80.f} * constants::f_cell_size;
		svc.data.switch_destructible_state(90301);
		++progress;
		break;
	case 10: camera_focus = sf::Vector2f{27.f, 80.f} * constants::f_cell_size; break;
	case 11: camera_focus = player.get_camera_focus_point(); break;
	case 12:
		camera_focus = player.get_camera_focus_point();
		if (!console.has_value()) {
			cooldowns.end.start();
			++progress;
		}
		break;
	case 13: camera_focus = player.get_camera_focus_point(); break;
	}

	svc.camera_controller.set_owner(graphics::CameraOwner::system);
	svc.camera_controller.set_position(camera_focus);
}

} // namespace fornani
