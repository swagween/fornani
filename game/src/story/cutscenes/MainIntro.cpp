
#include <fornani/entities/player/Player.hpp>
#include <fornani/gui/console/Console.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/story/cutscene/MainIntro.hpp>
#include <fornani/world/Map.hpp>

namespace fornani {

MainIntro::MainIntro(automa::ServiceProvider& svc, world::Map& map, player::Player& player) : Cutscene(svc, 300, "main_intro") {
	cooldowns.beginning.start();
	auto npcs = map.get_entities<NPC>();
	auto bit = std::ranges::find_if(npcs, [](auto& n) { return n->get_specifier() == 0; });
	auto& bryn = *bit;
	auto wit = std::ranges::find_if(npcs, [](auto& n) { return n->get_specifier() == 2; });
	auto& willett = *wit;

	bryn->set_flag(NPCFlags::cutscene);
	bryn->request(NPCAnimationState::inspect);
	bryn->set_direction(SimpleDirection{LR::right});
	willett->set_flag(NPCFlags::cutscene);

	svc.camera_controller.set_position(willett->Mobile::get_global_center());

	svc.music_player.load(svc.finder, "aether");
	svc.state_flags.set(automa::StateFlags::cutscene);
	player.set_idle();
}

void MainIntro::update(automa::ServiceProvider& svc, std::optional<std::unique_ptr<gui::Console>>& console, world::Map& map, player::Player& player) {

	static auto progress = util::Counter{};
	static auto ended = false;

	if (complete()) {
		player.controller.unrestrict();
		svc.state_flags.reset(automa::StateFlags::hide_hud);
		svc.state_flags.reset(automa::StateFlags::no_menu);
		svc.state_flags.reset(automa::StateFlags::cutscene);
		svc.camera_controller.set_owner(graphics::CameraOwner::player);
		map.transition.end();
		flags.set(CutsceneFlags::delete_me);
		svc.music_player.play_looped();
		return;
	}

	if (cooldowns.pause.is_almost_complete() && ended) { map.transition.start(); }
	if (cooldowns.end.is_almost_complete() && ended) { flags.set(CutsceneFlags::complete); }

	svc.state_flags.set(automa::StateFlags::hide_hud);
	svc.state_flags.set(automa::StateFlags::no_menu);
	svc.state_flags.set(automa::StateFlags::cutscene);
	cooldowns.pause.update();
	cooldowns.long_pause.update();
	cooldowns.beginning.update();

	if (map.transition.is(graphics::TransitionState::black)) { cooldowns.end.update(); }
	if (ended) { return; }

	if (console) { console.value()->set_no_exit(true); }

	auto npcs = map.get_entities<NPC>();
	auto bit = std::ranges::find_if(npcs, [](auto& n) { return n->get_specifier() == 0; });
	auto& bryn = *bit;
	auto wit = std::ranges::find_if(npcs, [](auto& n) { return n->get_specifier() == 2; });
	auto& willett = *wit;

	if (console.has_value()) { bryn->disengage(); }
	if (console.has_value()) { willett->disengage(); }

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

	static auto hit_stop = false;
	if (willett->get_collider().get_center().x / constants::f_cell_size < 26.5f) {
		willett->walk();
	} else {
		willett->request(NPCAnimationState::idle);
	}
	if (player.get_position().x / constants::f_cell_size > 25.f && !hit_stop) {
		player.set_idle();
		hit_stop = true;
	}

	player.set_direction(Direction{LR::right});

	auto total_suites{0};
	for (auto& npc : npcs) { total_suites += npc->get_number_of_suites(); }
	total_conversations = std::max(total_conversations, total_suites);
	if (cooldowns.end.is_almost_complete()) {
		flags.set(CutsceneFlags::complete);
		map.transition.end();
		return;
	}

	if (npcs.empty()) { return; }

	player.controller.restrict_movement();

	// get npcs
	if (cooldowns.end.running()) { bryn->disengage(); }

	svc.camera_controller.set_owner(graphics::CameraOwner::system);
	// svc.camera_controller.set_position(willett->Mobile::get_global_center());

	if (has_flag_set(MainIntroFlags::rumble)) {
		// svc.camera_controller.shake();
		// map.shake_camera();
		svc.camera_controller.shake(10, 0.3f, 200, 20);
		set_flag(MainIntroFlags::rumble, false);
	}

	if (!map.transition.is(graphics::TransitionState::inactive)) { return; }

	switch (progress.get_count()) {
	case 0:
		if (!console) { willett->force_engage(); }
		progress.update();
		return;
	case 1:
		if (!console) {
			willett->flush_conversations();
			willett->push_conversation(21);
			cooldowns.long_pause.start();
			svc.camera_controller.set_position(bryn->Mobile::get_global_center());
			progress.update();
			bryn->request(NPCAnimationState::turn);
			return;
		}
		break;
	case 2:
		if (cooldowns.long_pause.is_almost_complete()) { bryn->force_engage(); }
		cooldowns.long_pause.running() ? bryn->walk() : bryn->request(NPCAnimationState::idle);
		if (!console && !cooldowns.long_pause.running()) {
			bryn->flush_conversations();
			bryn->push_conversation(21);
			willett->force_engage();
			svc.camera_controller.set_position(willett->Mobile::get_global_center());
			progress.update();
			return;
		}
		break;
	case 3:
		if (!console) {
			willett->flush_conversations();
			willett->push_conversation(22);
			bryn->force_engage();
			svc.camera_controller.set_position(bryn->Mobile::get_global_center());
			progress.update();
			return;
		}
		break;
	case 4:
		if (!console) {
			bryn->pop_conversation();
			bryn->push_conversation(23);
			willett->force_engage();
			svc.camera_controller.set_position(willett->Mobile::get_global_center());
			set_flag(MainIntroFlags::rumble);
			progress.update();
			return;
		}
		break;
	case 5:
		if (!console) {
			willett->flush_conversations();
			willett->push_conversation(24);
			bryn->force_engage();
			svc.camera_controller.set_position(bryn->Mobile::get_global_center());
			progress.update();
			return;
		}
		break;
	case 6:
		if (!console) {
			bryn->pop_conversation();
			bryn->push_conversation(24);
			willett->force_engage();
			svc.camera_controller.set_position(willett->Mobile::get_global_center());
			set_flag(MainIntroFlags::rumble);
			progress.update();
			return;
		}
		break;
	case 7:
		if (!console) {
			willett->flush_conversations();
			willett->push_conversation(26);
			set_flag(MainIntroFlags::rumble);
			progress.update();
			bryn->force_engage();
			return;
		}
		break;
	case 8:
		if (!console) {
			bryn->pop_conversation();
			willett->force_engage();
			svc.camera_controller.set_position(willett->Mobile::get_global_center());
			set_flag(MainIntroFlags::rumble);
			progress.update();
			return;
		}
		break;
	case 9:
		if (!console && !ended) {
			cooldowns.end.start();
			cooldowns.pause.start();
			bryn->flush_conversations();
			willett->flush_conversations();
			willett->push_conversation(23);
			ended = true;
			return;
		}
		break;
	}
}

} // namespace fornani
