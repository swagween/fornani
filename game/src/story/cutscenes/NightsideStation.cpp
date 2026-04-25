
#include <fornani/automa/SceneContext.hpp>
#include <fornani/entities/player/Player.hpp>
#include <fornani/gui/console/Console.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/story/cutscene/NightsideStation.hpp>
#include <fornani/world/Map.hpp>

namespace fornani {

NightsideStation::NightsideStation(automa::ServiceProvider& svc) : Cutscene(svc, 901, "nightside_station"), m_intro{400} {
	m_intro.start();
	svc.music_player.stop();
	svc.music_player.load(svc.finder, "glitchified");
}

void NightsideStation::update(automa::ServiceProvider& svc, SceneContext& context, world::Map& map, player::Player& player) {
	if (complete()) {
		player.controller.unrestrict();
		svc.state_flags.reset(automa::StateFlags::hide_hud);
		svc.state_flags.reset(automa::StateFlags::no_menu);
		svc.state_flags.reset(automa::StateFlags::cutscene);
		svc.camera_controller.set_owner(graphics::CameraOwner::player);
		svc.music_player.stop();
		svc.music_player.load(svc.finder, "wind");
		svc.music_player.play_looped();
		svc.quest_table.progress_quest("open_nightside_station", 1, 901);
		flags.set(CutsceneFlags::delete_me);
		return;
	}

	svc.state_flags.set(automa::StateFlags::hide_hud);
	svc.state_flags.set(automa::StateFlags::no_menu);
	svc.state_flags.set(automa::StateFlags::cutscene);
	cooldowns.beginning.update();
	cooldowns.pause.update();
	cooldowns.long_pause.update();
	cooldowns.end.update();

	m_intro.update();
	if (m_intro.is_almost_complete()) { cooldowns.beginning.start(); }
	if (m_intro.running()) {
		if (m_intro.just_started()) { svc.soundboard.play_sound("phone_dial"); }
		return;
	}

	player.controller.restrict_movement();

	if (context.console) { context.console.value()->set_no_exit(true); }

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
		if (!context.console.has_value()) { bryn->force_engage(); }
	}
	if (context.console) { bryn->disengage(); }

	auto camera_focus = player.get_camera_focus_point();

	switch (progress) {
	case 1: camera_focus = sf::Vector2f{34.f, 14.f} * constants::f_cell_size; break;
	case 2:
		camera_focus = sf::Vector2f{34.f, 14.f} * constants::f_cell_size;
		svc.data.switch_destructible_state(90101);
		++progress;
		break;
	case 3: camera_focus = sf::Vector2f{34.f, 14.f} * constants::f_cell_size; break;
	case 4:
		camera_focus = player.get_camera_focus_point();
		if (!context.console.has_value()) {
			cooldowns.end.start();
			++progress;
		}
		break;
	case 5: camera_focus = player.get_camera_focus_point(); break;
	}

	svc.camera_controller.set_owner(graphics::CameraOwner::system);
	svc.camera_controller.set_position(camera_focus);
}

} // namespace fornani
