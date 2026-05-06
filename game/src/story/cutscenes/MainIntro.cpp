
#include <fornani/automa/SceneContext.hpp>
#include <fornani/entities/player/Player.hpp>
#include <fornani/gui/console/Console.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/story/cutscene/MainIntro.hpp>
#include <fornani/world/Map.hpp>

namespace fornani {

MainIntro::MainIntro(automa::ServiceProvider& svc, world::Map& map, player::Player& player) : Cutscene(svc, 300, "main_intro"), m_outro{2000}, m_willett_walk{400} {
	cooldowns.beginning.start();
	auto npcs = map.get_entities<NPC>();
	auto bit = std::ranges::find_if(npcs, [](auto& n) { return n->get_specifier() == 0; });
	auto& bryn = *bit;
	auto wit = std::ranges::find_if(npcs, [](auto& n) { return n->get_specifier() == 2; });
	auto& willett = *wit;

	bryn->set_direction(SimpleDirection{LR::right});
	bryn->set_flag(NPCFlags::cutscene);
	willett->set_flag(NPCFlags::cutscene);
	bryn->request(NPCAnimationState::busy);
	player.set_flag(player::PlayerFlags::cutscene);

	svc.music_player.load(svc.finder, "aether");
	svc.ambience_player.tracks.open.fade_in(util::Sec{2.f});
	svc.state_flags.set(automa::StateFlags::cutscene);
	player.set_idle();

	player.set_direction(Direction{LR::right});
}

void MainIntro::update(automa::ServiceProvider& svc, SceneContext& context, world::Map& map, player::Player& player) {
	if (complete()) {
		player.controller.unrestrict();
		svc.state_flags.reset(automa::StateFlags::hide_hud);
		svc.state_flags.reset(automa::StateFlags::no_menu);
		svc.state_flags.reset(automa::StateFlags::cutscene);
		svc.camera_controller.set_owner(graphics::CameraOwner::player);
		flags.set(CutsceneFlags::delete_me);
		player.set_flag(player::PlayerFlags::cutscene, false);
		return;
	}

	if (m_outro.is_almost_complete()) { flags.set(CutsceneFlags::complete); }

	svc.state_flags.set(automa::StateFlags::hide_hud);
	svc.state_flags.set(automa::StateFlags::no_menu);
	svc.state_flags.set(automa::StateFlags::cutscene);
	cooldowns.pause.update();
	cooldowns.long_pause.update();
	cooldowns.beginning.update();
	m_outro.update();
	m_willett_walk.update();

	if (context.console) { context.console.value()->set_no_exit(true); }

	auto npcs = map.get_entities<NPC>();
	auto bit = std::ranges::find_if(npcs, [](auto& n) { return n->get_specifier() == 0; });
	auto& bryn = *bit;
	auto wit = std::ranges::find_if(npcs, [](auto& n) { return n->get_specifier() == 2; });
	auto& willett = *wit;

	if (context.console.has_value()) { bryn->disengage(); }
	if (context.console.has_value()) { willett->disengage(); }

	if (!flags.test(CutsceneFlags::started)) {
		bryn->flush_conversations();
		willett->flush_conversations();
		bryn->push_conversation(20);
		willett->push_conversation(20);
		svc.music_player.play_looped();
		player.set_slow_walk();
		flags.set(CutsceneFlags::started);
		willett->set_direction(LR::right);
	}

	if (willett->get_collider().get_center().x / constants::f_cell_size < 27.25f && progress < 6) {
		willett->walk();
	} else if (progress < 20) {
		willett->face_player(player);
		willett->request(NPCAnimationState::idle);
	}
	if (player.get_position().x / constants::f_cell_size > 25.4f && !has_flag_set(MainIntroFlags::player_stopped)) {
		player.set_idle();
		set_flag(MainIntroFlags::player_stopped);
	}

	auto total_suites{0};
	for (auto& npc : npcs) { total_suites += npc->get_number_of_suites(); }
	total_conversations = std::max(total_conversations, total_suites);

	if (npcs.empty()) { return; }

	player.controller.restrict_movement();
	player.stall_idle_timer();

	svc.camera_controller.set_owner(graphics::CameraOwner::system);
	svc.camera_controller.free();
	if (has_flag_set(MainIntroFlags::rumble)) {
		svc.camera_controller.shake(10, 0.5f, 700, 50);
		svc.soundboard.flags.world.set(audio::World::delay_crash);
		svc.soundboard.flags.world.set(audio::World::vibration);
		player.set_hurt();
		set_flag(MainIntroFlags::rumble, false);
		bryn->request(NPCAnimationState::stagger);
		svc.music_player.stop();
		svc.ticker.freeze_frame(10);
	}
	if (has_flag_set(MainIntroFlags::takeover)) {
		if (has_flag_set(MainIntroFlags::start_takeover)) {
			svc.ambience_player.load(svc.finder, "intro_takeover");
			svc.ambience_player.play();
			set_flag(MainIntroFlags::start_takeover, false);
		}
		svc.ambience_player.set_balance(m_outro.get_normalized());
	}

	if (!context.transition.is(graphics::TransitionState::inactive)) {
		svc.camera_controller.set_position(willett->Mobile::get_global_center());
		return;
	}

	if (progress > 1) { bryn->start_busy_timer(); }
	switch (progress) {
	case 0:
		if (!context.console) { willett->force_engage(); }
		++progress;
		break;
	case 1:
		svc.camera_controller.set_position(willett->Mobile::get_global_center());
		if (!context.console) {
			willett->flush_conversations();
			willett->push_conversation(21);
			cooldowns.long_pause.start(360);
			svc.camera_controller.set_position(bryn->Mobile::get_global_center());
			++progress;
			bryn->set_desired_direction({LR::left});
			bryn->start_busy_timer();
			bryn->request(NPCAnimationState::turn);
		}
		break;
	case 2:
		if (cooldowns.long_pause.is_almost_complete()) { bryn->force_engage(); }
		cooldowns.long_pause.running() ? bryn->walk() : bryn->request(NPCAnimationState::idle);
		svc.camera_controller.set_position(bryn->Mobile::get_global_center());
		if (!context.console && !cooldowns.long_pause.running()) {
			bryn->flush_conversations();
			bryn->push_conversation(21);
			willett->force_engage();
			svc.camera_controller.set_position(willett->Mobile::get_global_center());
			++progress;
		}
		break;
	case 3:
		svc.camera_controller.set_position(willett->Mobile::get_global_center());
		if (!context.console) {
			willett->flush_conversations();
			willett->push_conversation(22);
			bryn->force_engage();
			svc.camera_controller.set_position(bryn->Mobile::get_global_center());
			++progress;
			return;
		}
		break;
	case 4:
		svc.camera_controller.set_position(bryn->Mobile::get_global_center());
		if (!context.console) {
			bryn->flush_conversations();
			bryn->push_conversation(22);
			willett->force_engage();
			svc.camera_controller.set_position(willett->Mobile::get_global_center());
			++progress;
		}
		break;
	case 5:
		// got to 20 after this
		svc.camera_controller.set_position(willett->Mobile::get_global_center());
		if (!context.console) {
			willett->flush_and_push(26);
			m_willett_walk.start();
			progress = 20;
		}
		break;
	case 6:
		svc.camera_controller.set_position(bryn->Mobile::get_global_center());
		if (!context.console) {
			bryn->flush_conversations();
			bryn->push_conversation(23);
			willett->force_engage();
			player.turn();
			svc.camera_controller.set_position(willett->Mobile::get_global_center());
			++progress;
		}
		break;
	case 7:
		svc.camera_controller.set_position(willett->Mobile::get_global_center());
		if (!context.console) {
			willett->flush_conversations();
			willett->push_conversation(24);
			++progress;
			bryn->force_engage();
			player.turn();
		}
		break;
	case 8:
		svc.camera_controller.set_position(bryn->Mobile::get_global_center());
		if (!context.console) {
			bryn->flush_conversations();
			bryn->push_conversation(24);
			willett->force_engage();
			player.turn();
			svc.camera_controller.set_position(willett->Mobile::get_global_center());
			++progress;
		}
		break;
	case 9:
		svc.camera_controller.set_position(willett->Mobile::get_global_center());
		if (!context.console) {
			willett->flush_conversations();
			willett->push_conversation(25);
			set_flag(MainIntroFlags::rumble);
			++progress;
			cooldowns.long_pause.start();
		}
		break;
	case 10:
		if (cooldowns.long_pause.is_almost_complete()) { bryn->force_engage(); }
		svc.camera_controller.set_position(bryn->Mobile::get_global_center());
		if (!context.console && !cooldowns.long_pause.running()) {
			bryn->flush_conversations();
			willett->force_engage();
			svc.camera_controller.set_position(willett->Mobile::get_global_center());
			++progress;
		}
		break;
	case 11:
		svc.camera_controller.set_position(willett->Mobile::get_global_center());
		if (!context.console) {
			willett->flush_conversations();
			++progress;
			set_flag(MainIntroFlags::rumble);
		}
		break;
	case 12:
		svc.camera_controller.set_position(willett->Mobile::get_global_center());
		if (!context.console && !has_flag_set(MainIntroFlags::takeover)) {
			bryn->flush_conversations();
			willett->flush_conversations();
			set_flag(MainIntroFlags::takeover);
			set_flag(MainIntroFlags::start_takeover);
			context.transition.start();
			m_outro.start();
		}
		break;
	case 20:
		willett->walk();
		svc.camera_controller.set_position(willett->Mobile::get_global_center());
		if (m_willett_walk.is_complete()) {
			willett->set_desired_direction({LR::right});
			willett->set_flag(NPCFlags::face_player);
			willett->Mobile::set_animation("turn");
			willett->force_engage();
			player.turn();
			++progress;
		}
		break;
	case 21:
		svc.camera_controller.set_position(willett->Mobile::get_global_center());
		if (!context.console) {
			willett->flush_and_push(23);
			bryn->force_engage();
			player.turn();
			progress = 6;
		}
		break;
	}
}

} // namespace fornani
