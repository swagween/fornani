
#include <fornani/automa/SceneContext.hpp>
#include <fornani/entities/player/Player.hpp>
#include <fornani/gui/console/Console.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/story/cutscene/SpencerReveal.hpp>
#include <fornani/world/Map.hpp>

namespace fornani {

SpencerReveal::SpencerReveal(automa::ServiceProvider& svc) : Cutscene(svc, 1310, "spencer_reveal"), m_intro{20} {
	cooldowns.beginning.start();
	m_intro.start();
	svc.input_system.flush_inputs();
	svc.state_flags.set(automa::StateFlags::cutscene);
}

void SpencerReveal::update(automa::ServiceProvider& svc, SceneContext& context, world::Map& map, player::Player& player) {
	if (complete()) {
		player.controller.unrestrict();
		svc.state_flags.reset(automa::StateFlags::hide_hud);
		svc.state_flags.reset(automa::StateFlags::no_menu);
		svc.state_flags.reset(automa::StateFlags::cutscene);
		svc.camera_controller.set_owner(graphics::CameraOwner::player);
		svc.camera_controller.constrain();
		svc.quest_table.progress_quest("ashtown_bandit", 1, 1310);
		svc.quest_table.progress_quest("find_spencer", 1, 1310);
		svc.music_player.resume();
		player.set_flag(player::PlayerFlags::cutscene, false);
		flags.set(CutsceneFlags::delete_me);
		return;
	}

	svc.state_flags.set(automa::StateFlags::hide_hud);
	svc.state_flags.set(automa::StateFlags::no_menu);
	svc.state_flags.set(automa::StateFlags::cutscene);
	svc.camera_controller.free();
	cooldowns.beginning.update();
	cooldowns.pause.update();
	cooldowns.long_pause.update();
	cooldowns.end.update();

	player.controller.restrict_movement();
	player.stall_idle_timer();
	player.set_flag(player::PlayerFlags::show_weapon, false);

	if (context.console) { context.console.value()->set_no_exit(true); }

	auto npcs = map.get_entities<NPC>();
	auto bit = std::ranges::find_if(npcs, [](auto& n) { return n->get_specifier() == 42; });
	auto& bandit = *bit;
	auto sit = std::ranges::find_if(npcs, [](auto& n) { return n->get_specifier() == 6; });
	auto& spencer = *sit;

	if (cooldowns.end.is_almost_complete()) {
		flags.set(CutsceneFlags::complete);
		return;
	}

	if (m_intro.just_started()) {
		player.controller.prevent_movement();
		player.get_collider().physics.zero_x();
		player.set_flag(player::PlayerFlags::cutscene);
		if (player.get_actual_direction().left()) { player.set_direction(Direction{LR::right}); }
		player.set_slow_walk();
		spencer->set_flag(NPCFlags::cutscene);
		bandit->set_flag(NPCFlags::cutscene);
	}
	m_intro.update();
	if (m_intro.running()) { return; }

	if (cooldowns.end.running()) { bandit->disengage(); }
	if (context.console) { bandit->disengage(); }
	if (cooldowns.beginning.is_almost_complete()) {}
	if (player.get_collider().get_vicinity_rect().contains(bandit->get_collider().get_center()) && !m_flags.test(SpencerRevealFlags::player_stopped)) {
		player.set_idle();
		m_flags.set(SpencerRevealFlags::player_stopped);
		if (!context.console.has_value()) { bandit->force_engage(); }
	}

	auto camera_focus = bandit->get_collider().get_center();

	switch (progress) {
	case 1:
		if (!context.console) {
			cooldowns.end.start();
			++progress;
		}
		break;
	case 2: break;
	case 3: break;
	}

	svc.camera_controller.set_owner(graphics::CameraOwner::system);
	svc.camera_controller.set_position(camera_focus);
}

} // namespace fornani
