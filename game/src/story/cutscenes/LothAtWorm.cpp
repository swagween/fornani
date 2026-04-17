
#include <fornani/entities/player/Player.hpp>
#include <fornani/gui/console/Console.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/story/cutscene/LothAtWorm.hpp>
#include <fornani/world/Map.hpp>

namespace fornani {

LothAtWorm::LothAtWorm(automa::ServiceProvider& svc) : Cutscene(svc, 268, "loth_at_worm"), m_intro{200} {
	m_intro.start();
	svc.music_player.load(svc.finder, "looking_glass");
	svc.input_system.flush_inputs();
	svc.state_flags.set(automa::StateFlags::cutscene);
}

void LothAtWorm::update(automa::ServiceProvider& svc, std::optional<std::unique_ptr<gui::Console>>& console, world::Map& map, player::Player& player) {
	if (complete()) {
		player.controller.unrestrict();
		svc.state_flags.reset(automa::StateFlags::hide_hud);
		svc.state_flags.reset(automa::StateFlags::no_menu);
		svc.state_flags.reset(automa::StateFlags::cutscene);
		svc.camera_controller.set_owner(graphics::CameraOwner::player);
		svc.music_player.stop();
		svc.music_player.load(svc.finder, "none");
		svc.quest_table.progress_quest("mystery_of_pixiote", 1, 26801);
		svc.music_player.play_looped();
		flags.set(CutsceneFlags::delete_me);
		player.set_flag(player::PlayerFlags::cutscene, false);
		svc.camera_controller.constrain();
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
	player.set_flag(player::PlayerFlags::show_weapon, false);

	auto npcs = map.get_entities<NPC>();
	auto lit = std::ranges::find_if(npcs, [](auto& n) { return n->get_specifier() == 37; });
	auto& loth = *lit;

	if (m_intro.just_started()) {
		player.controller.prevent_movement();
		player.get_collider().physics.zero_x();
		player.set_flag(player::PlayerFlags::cutscene);
		if (player.get_actual_direction().left()) { player.set_direction(Direction{LR::right}); }
		player.set_slow_walk();
		loth->set_flag(NPCFlags::cutscene);
	}
	if (m_intro.is_almost_complete()) { cooldowns.beginning.start(); }
	m_intro.update();
	if (m_intro.running()) { return; }

	if (console) { console.value()->set_no_exit(true); }

	auto total_suites{0};
	for (auto& npc : npcs) { total_suites += npc->get_number_of_suites(); }
	total_conversations = std::max(total_conversations, total_suites);
	if (cooldowns.end.is_almost_complete()) {
		flags.set(CutsceneFlags::complete);
		return;
	}
	if (npcs.empty()) { return; }

	if (cooldowns.end.running()) { loth->disengage(); }
	if (cooldowns.beginning.is_almost_complete()) {}
	if (console) { loth->disengage(); }
	if (player.get_collider().get_vicinity_rect().contains(loth->get_collider().get_center()) && !m_flags.test(LothAtWormFlags::player_stopped)) {
		player.set_idle();
		m_flags.set(LothAtWormFlags::player_stopped);
		if (!console.has_value()) { loth->force_engage(); }
	}
	if (m_flags.test(LothAtWormFlags::player_stopped)) {}

	svc.camera_controller.set_owner(graphics::CameraOwner::system);
	svc.camera_controller.free();
	svc.camera_controller.set_position(loth->Mobile::get_global_center());

	switch (progress) {
	case 1:
		if (!console) {
			loth->flush_conversations();
			loth->push_conversation(2);
			cooldowns.long_pause.start(256);
			loth->set_desired_direction(LR::left);
			++progress;
			return;
		}
		break;
	case 2:
		if (cooldowns.long_pause.is_almost_complete()) {
			loth->force_engage();
			svc.music_player.play_looped();
			++progress;
		}
		break;
	case 3:
		if (!console) {
			loth->flush_conversations();
			loth->push_conversation(3);
			loth->request(NPCAnimationState::inspect);
			cooldowns.long_pause.start(256);
			++progress;
			return;
		}
		break;
	case 4:
		loth->walk();
		++progress;
		break;
	case 5:
		if (loth->get_collider().get_center().x < player.get_center().x && !m_flags.test(LothAtWormFlags::nani_turned)) {
			player.turn();
			m_flags.set(LothAtWormFlags::nani_turned);
		}
		if (cooldowns.long_pause.is_almost_complete()) {
			loth->request(NPCAnimationState::busy);
			loth->force_engage();
			++progress;
		}
		break;
	case 7:
		svc.music_player.stop();
		svc.music_player.load(svc.finder, "spiral_glass");
		loth->set_desired_direction(LR::right);
		++progress;
		break;
	case 9:
		svc.music_player.play_looped();
		++progress;
		break;
	case 11:
		if (!console) {
			loth->request(NPCAnimationState::special_1);
			svc.soundboard.play_sound("magical_teleport");
			++progress;
		}
		break;
	case 12:
		if (loth->is_animation_complete()) {
			cooldowns.end.start();
			loth->hide();
			svc.soundboard.play_sound("magical_sparkle");
			map.spawn_emitter(svc, "radiance", loth->get_collider().get_center(), Direction{});
			map.spawn_effect(svc, "giga_flare", loth->get_collider().get_center());
			++progress;
		}
		break;
	}
}

} // namespace fornani
