
#include <fornani/automa/SceneContext.hpp>
#include <fornani/entities/player/Player.hpp>
#include <fornani/gui/console/Console.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/story/cutscene/SpencerReveal.hpp>
#include <fornani/world/Map.hpp>

namespace fornani {

SpencerReveal::SpencerReveal(automa::ServiceProvider& svc) : Cutscene(svc, 1310, "spencer_reveal"), m_intro{20}, m_spencer_reveal{420}, m_dial{800} {
	cooldowns.beginning.start();
	m_intro.start();
	svc.input_system.flush_inputs();
	svc.state_flags.set(automa::StateFlags::cutscene);
	if (svc.quest_table.get_quest_progression("find_spencer") > 1) { progress = 4; }
}

void SpencerReveal::update(automa::ServiceProvider& svc, SceneContext& context, world::Map& map, player::Player& player) {
	if (complete()) {
		player.controller.unrestrict();
		svc.state_flags.reset(automa::StateFlags::hide_hud);
		svc.state_flags.reset(automa::StateFlags::no_menu);
		svc.state_flags.reset(automa::StateFlags::cutscene);
		svc.camera_controller.set_owner(graphics::CameraOwner::player);
		svc.camera_controller.constrain();
		if (progress > 40) {
			svc.quest_table.set_quest_progression("find_spencer", 11);
			svc.quest_table.set_quest_progression("the_hoarder", 1);
			svc.quest_table.set_quest_progression("ashtown_bandit", 11);
		} else {
			svc.quest_table.progress_quest("find_spencer", 1, 1310);
		}
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
	m_spencer_reveal.update();
	m_dial.update();

	player.controller.restrict_movement();
	if (progress < 43) { player.stall_idle_timer(); }
	player.set_flag(player::PlayerFlags::show_weapon, false);

	if (context.console) { context.console.value()->set_no_exit(true); }

	auto npcs = map.get_entities<NPC>();
	auto bit = std::ranges::find_if(npcs, [](auto& n) { return n->get_specifier() == 42; });
	auto& bandit = *bit;
	auto sit = std::ranges::find_if(npcs, [](auto& n) { return n->get_specifier() == 6; });
	auto& spencer = *sit;
	auto brit = std::ranges::find_if(npcs, [](auto& n) { return n->get_specifier() == 0; });
	auto& bryn = *brit;
	auto dit = std::ranges::find_if(npcs, [](auto& n) { return n->get_specifier() == 2; });
	auto& doc = *dit;

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

	if (cooldowns.end.running()) {
		bandit->disengage();
		spencer->disengage();
	}
	if (context.console) {
		bandit->disengage();
		spencer->disengage();
		doc->disengage();
		bryn->disengage();
	}
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
	case 4:
		bandit->flush_and_push(6);
		++progress;
		break;
	case 10:
		if (!context.console) {
			bandit->hide();
			spencer->unhide();
			spencer->set_special_animation(3);
			svc.music_player.stop();
			m_spencer_reveal.start();
			++progress;
		}
		break;
	case 11:
		if (m_spencer_reveal.is_almost_complete()) {
			spencer->flush_and_push(1);
			spencer->force_engage();
			svc.music_player.resume();
			++progress;
		}
		break;
	case 20:
		if (!context.console) {
			context.transition.start();
			context.transition.hang();
			svc.music_player.stop();
			++progress;
		}
		break;
	case 21:
		if (context.transition.is_black() && context.transition.has_waited(400)) {
			spencer->set_busy();
			context.transition.end();
			++progress;
		}
		break;
	case 22:
		if (context.transition.is(graphics::TransitionState::inactive)) {
			spencer->flush_and_push(2);
			spencer->force_engage();
			svc.music_player.resume();
			++progress;
		}
		break;
	case 25:
		if (!context.console) {
			svc.soundboard.play_sound("phone_ring_outgoing");
			m_dial.start();
			++progress;
		}
		break;
	case 26:
		if (m_dial.is_almost_complete()) {
			svc.soundboard.play_sound("radio_hang_up");
			map.reveal_npc("dr_willett");
			doc->set_invisible();
			doc->flush_and_push(30);
			doc->force_engage();
			++progress;
		}
		break;
	case 27:
		if (context.console) { context.console.value()->set_hologram(); }
		if (!context.console) {
			spencer->flush_and_push(3);
			spencer->force_engage();
			++progress;
		}
		break;
	case 28:
		if (context.console) { context.console.value()->set_hologram(false); }
		if (!context.console) {
			doc->flush_and_push(31);
			doc->force_engage();
			++progress;
		}
		break;
	case 29:
		if (context.console) { context.console.value()->set_hologram(); }
		if (!context.console) {
			spencer->flush_and_push(4);
			spencer->force_engage();
			++progress;
		}
		break;
	case 30:
		if (context.console) { context.console.value()->set_hologram(false); }
		if (!context.console) {
			doc->flush_and_push(32);
			doc->force_engage();
			++progress;
		}
		break;
	case 31:
		if (context.console) { context.console.value()->set_hologram(); }
		if (!context.console) {
			spencer->flush_and_push(5);
			spencer->force_engage();
			++progress;
		}
		break;
	case 32:
		if (context.console) { context.console.value()->set_hologram(false); }
		if (!context.console) {
			doc->flush_and_push(33);
			doc->force_engage();
			++progress;
		}
		break;
	case 33:
		if (context.console) { context.console.value()->set_hologram(); }
		if (!context.console) {
			spencer->flush_and_push(6);
			spencer->force_engage();
			++progress;
		}
		break;
	case 34:
		if (context.console) { context.console.value()->set_hologram(false); }
		if (!context.console) {
			map.reveal_npc("bryn");
			bryn->set_invisible();
			bryn->flush_and_push(31);
			bryn->force_engage();
			++progress;
		}
		break;
	case 35:
		if (context.console) { context.console.value()->set_hologram(); }
		if (!context.console) {
			spencer->flush_and_push(7);
			spencer->force_engage();
			++progress;
		}
		break;
	case 36:
		if (context.console) { context.console.value()->set_hologram(false); }
		if (!context.console) {
			bryn->flush_and_push(32);
			bryn->force_engage();
			++progress;
		}
		break;
	case 37:
		if (context.console) { context.console.value()->set_hologram(); }
		if (!context.console) {
			spencer->flush_and_push(8);
			spencer->force_engage();
			++progress;
		}
		break;
	case 38:
		if (context.console) { context.console.value()->set_hologram(false); }
		if (!context.console) {
			bryn->flush_and_push(33);
			bryn->force_engage();
			++progress;
		}
		break;
	case 39:
		if (context.console) { context.console.value()->set_hologram(); }
		if (!context.console) {
			svc.soundboard.play_sound("radio_hang_up");
			spencer->flush_and_push(9);
			cooldowns.pause.start(300);
			doc->hide();
			bryn->hide();
			++progress;
		}
		break;
	case 40:
		if (cooldowns.pause.is_almost_complete()) {
			spencer->force_engage();
			++progress;
		}
		break;
	case 41:
		if (context.console) { context.console.value()->set_hologram(false); }
		if (!context.console) {
			context.transition.start();
			context.transition.hang();
			svc.music_player.stop();
			++progress;
		}
		break;
	case 42:
		if (context.transition.is_black() && context.transition.has_waited(400)) {
			spencer->request(NPCAnimationState::idle);
			context.transition.end();
			++progress;
		}
		break;
	case 43:
		if (context.transition.is(graphics::TransitionState::inactive)) {
			spencer->flush_and_push(10);
			spencer->force_engage();
			svc.music_player.load(svc.finder, "wind");
			svc.music_player.load(svc.finder, "ashtown");
			svc.music_player.play_looped();
			++progress;
		}
		break;
	case 44:
		if (!context.console) {
			cooldowns.end.start();
			spencer->flush_and_push(11);
			++progress;
		}
		break;
	}

	svc.camera_controller.set_owner(graphics::CameraOwner::system);
	svc.camera_controller.set_position(camera_focus);
}

} // namespace fornani
