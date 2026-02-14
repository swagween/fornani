
#include <fornani/entities/player/Player.hpp>
#include <fornani/gui/console/Console.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/story/cutscene/PioneerBaseDebrief.hpp>
#include <fornani/world/Map.hpp>

namespace fornani {

PioneerBaseDebrief::PioneerBaseDebrief(automa::ServiceProvider& svc, world::Map& map, player::Player& player) : Cutscene(svc, 300, "pioneer_base_debrief") {
	cooldowns.beginning.start();
	auto npcs = map.get_entities<NPC>();
	auto bit = std::ranges::find_if(npcs, [](auto& n) { return n->get_specifier() == 0; });
	auto& bryn = *bit;
	auto wit = std::ranges::find_if(npcs, [](auto& n) { return n->get_specifier() == 2; });
	auto& willett = *wit;

	svc.camera_controller.set_position(willett->Mobile::get_global_center());
	map.transition.start();

	svc.music_player.load(svc.finder, "glitchified");
}

void PioneerBaseDebrief::update(automa::ServiceProvider& svc, std::optional<std::unique_ptr<gui::Console>>& console, world::Map& map, player::Player& player) {

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
		svc.quest_table.progress_quest("defeat_skycorps", 1, 50901);
		svc.music_player.load(svc.finder, "bryns_turn");
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

	if (map.transition.is(graphics::TransitionState::black)) {
		cooldowns.beginning.update();
		cooldowns.end.update();
	}
	if (ended) { return; }

	player.controller.restrict_movement();

	if (console) { console.value()->set_no_exit(true); }

	auto npcs = map.get_entities<NPC>();
	auto bit = std::ranges::find_if(npcs, [](auto& n) { return n->get_specifier() == 0; });
	auto& bryn = *bit;
	auto wit = std::ranges::find_if(npcs, [](auto& n) { return n->get_specifier() == 2; });
	auto& willett = *wit;

	if (map.transition.is(graphics::TransitionState::black) && cooldowns.beginning.is_complete()) {
		map.transition.end();
		svc.music_player.play_looped();
		bryn->set_position_from_scaled({10.f, 16.f});
		willett->set_position_from_scaled({12.f, 16.f});
		player.set_position(sf::Vector2f{11.f, 16.f} * constants::f_cell_size);
		player.set_idle();
		bryn->request(NPCAnimationState::inspect);
	}

	if (console.has_value()) { bryn->disengage(); }
	if (console.has_value()) { willett->disengage(); }

	if (!flags.test(CutsceneFlags::started)) {
		bryn->flush_conversations();
		willett->flush_conversations();
		bryn->push_conversation(10);
		willett->push_conversation(10);
		flags.set(CutsceneFlags::started);
	}

	auto total_suites{0};
	for (auto& npc : npcs) { total_suites += npc->get_number_of_suites(); }
	total_conversations = std::max(total_conversations, total_suites);
	if (cooldowns.end.is_almost_complete()) {
		flags.set(CutsceneFlags::complete);
		map.transition.end();
		return;
	}
	if (npcs.empty()) { return; }
	if (cooldowns.beginning.running()) {
		player.controller.prevent_movement();
		player.controller.set_direction(Direction{UND::neutral, LNR::left});
		return;
	}

	// get npcs
	if (cooldowns.end.running()) { bryn->disengage(); }
	if (bryn->get_collider().bounding_box.overlaps(player.get_collider().get_vicinity_rect())) {}

	svc.camera_controller.set_owner(graphics::CameraOwner::system);

	if (!map.transition.is(graphics::TransitionState::inactive)) { return; }

	switch (progress.get_count()) {
	case 0:
		if (!console) { willett->force_engage(); }
		progress.update();
		return;
	case 1:
		if (!console) {
			willett->flush_conversations();
			willett->push_conversation(11);
			bryn->force_engage();
			svc.camera_controller.set_position(bryn->Mobile::get_global_center());
			progress.update();
			return;
		}
		break;
	case 2:
		if (!console) {
			bryn->flush_conversations();
			bryn->push_conversation(11);
			willett->force_engage();
			svc.camera_controller.set_position(willett->Mobile::get_global_center());
			progress.update();
			return;
		}
		break;
	case 3:
		if (!console) {
			willett->flush_conversations();
			willett->push_conversation(12);
			bryn->force_engage();
			svc.camera_controller.set_position(bryn->Mobile::get_global_center());
			progress.update();
			return;
		}
		break;
	case 4:
		if (!console) {
			bryn->pop_conversation();
			willett->force_engage();
			svc.camera_controller.set_position(willett->Mobile::get_global_center());
			progress.update();
			return;
		}
		break;
	case 5:
		if (!console && !ended) {
			cooldowns.end.start();
			cooldowns.pause.start();
			bryn->flush_conversations();
			willett->flush_conversations();
			bryn->push_conversation(6);
			willett->push_conversation(13);
			ended = true;
			return;
		}
		break;
	}
}

} // namespace fornani
