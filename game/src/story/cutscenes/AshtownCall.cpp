
#include <fornani/automa/SceneContext.hpp>
#include <fornani/entities/player/Player.hpp>
#include <fornani/gui/console/Console.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/story/cutscene/AshtownCall.hpp>
#include <fornani/world/Map.hpp>

namespace fornani {

AshtownCall::AshtownCall(automa::ServiceProvider& svc) : Cutscene(svc, 1300, "ashtown_call"), m_intro{400} {
	m_intro.start();
	svc.music_player.stop();
	svc.music_player.load(svc.finder, "glitchified");
}

void AshtownCall::update(automa::ServiceProvider& svc, SceneContext& context, world::Map& map, player::Player& player) {
	if (complete()) {
		player.controller.unrestrict();
		svc.state_flags.reset(automa::StateFlags::hide_hud);
		svc.state_flags.reset(automa::StateFlags::no_menu);
		svc.state_flags.reset(automa::StateFlags::cutscene);
		svc.camera_controller.set_owner(graphics::CameraOwner::player);
		svc.camera_controller.constrain();
		svc.music_player.stop();
		svc.music_player.load(svc.finder, "i_dream_of_sky");
		svc.music_player.play_looped();
		svc.quest_table.progress_quest("find_spencer", 1, 1300);
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
	player.controller.restrict_movement();
	player.stall_idle_timer();

	m_intro.update();
	if (m_intro.is_almost_complete()) { cooldowns.beginning.start(); }
	if (m_intro.running()) {
		if (m_intro.just_started()) { svc.soundboard.play_sound("phone_dial"); }
		return;
	}

	if (context.console) { context.console.value()->set_no_exit(true); }

	auto npcs = map.get_entities<NPC>();
	auto bit = std::ranges::find_if(npcs, [](auto& n) { return n->get_specifier() == 0; });
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
		map.reveal_npc("bryn");
		bryn->set_invisible();
		svc.music_player.play_looped();
		if (!context.console.has_value()) { bryn->force_engage(); }
	}
	if (context.console) {
		bryn->disengage();
		context.console.value()->set_hologram();
	}

	auto camera_focus = player.get_camera_focus_point();

	switch (progress) {
	case 1:
		cooldowns.end.start();
		if (!context.console) {
			svc.soundboard.play_sound("radio_hang_up");
			++progress;
			bryn->hide();
		}
		break;
	}

	svc.camera_controller.set_owner(graphics::CameraOwner::system);
	svc.camera_controller.set_position(camera_focus);
}

} // namespace fornani
