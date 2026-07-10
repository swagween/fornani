
#include <fornani/automa/SceneContext.hpp>
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
	m_flags.set(PioneerBaseDebriefFlags::start);
}

void PioneerBaseDebrief::update(automa::ServiceProvider& svc, SceneContext& context, world::Map& map, player::Player& player) {

	if (m_flags.consume(PioneerBaseDebriefFlags::start)) { context.transition.start(); }

	if (complete()) {
		player.controller.unrestrict();
		svc.state_flags.reset(automa::StateFlags::hide_hud);
		svc.state_flags.reset(automa::StateFlags::no_menu);
		svc.state_flags.reset(automa::StateFlags::cutscene);
		svc.camera_controller.set_owner(graphics::CameraOwner::player);
		context.transition.end();
		flags.set(CutsceneFlags::delete_me);
		svc.quest_table.progress_quest("defeat_skycorps", 1, 50901);
		svc.music_player.load(svc.finder, "bryns_turn");
		player.set_flag(player::PlayerFlags::cutscene, false);
		svc.music_player.play_looped();
		return;
	}

	if (cooldowns.pause.is_almost_complete() && m_flags.test(PioneerBaseDebriefFlags::end)) { context.transition.start(); }
	if (cooldowns.end.is_almost_complete() && m_flags.test(PioneerBaseDebriefFlags::end)) { flags.set(CutsceneFlags::complete); }

	svc.state_flags.set(automa::StateFlags::hide_hud);
	svc.state_flags.set(automa::StateFlags::no_menu);
	svc.state_flags.set(automa::StateFlags::cutscene);
	cooldowns.pause.update();
	cooldowns.long_pause.update();

	if (context.transition.is(graphics::TransitionState::black)) {
		cooldowns.beginning.update();
		cooldowns.end.update();
	}
	if (m_flags.test(PioneerBaseDebriefFlags::end)) { return; }

	player.controller.restrict_movement();
	player.stall_idle_timer();

	if (context.console) { context.console.value()->set_no_exit(true); }

	auto npcs = map.get_entities<NPC>();
	auto bit = std::ranges::find_if(npcs, [](auto& n) { return n->get_specifier() == 0; });
	auto& bryn = *bit;
	auto wit = std::ranges::find_if(npcs, [](auto& n) { return n->get_specifier() == 2; });
	auto& willett = *wit;

	if (context.transition.is(graphics::TransitionState::black) && cooldowns.beginning.is_complete()) {
		context.transition.end();
		svc.music_player.load(svc.finder, "glitchified");
		svc.music_player.play_looped();
		bryn->set_position_from_scaled({10.f, 16.f});
		willett->set_position_from_scaled({12.f, 16.f});
		player.set_position_on_grid({11, 16});
		player.set_idle();
		player.set_direction({LR::right});
		player.set_flag(player::PlayerFlags::cutscene);
		bryn->request(NPCAnimationState::inspect);
	}

	if (context.console.has_value()) { bryn->disengage(); }
	if (context.console.has_value()) { willett->disengage(); }

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
		context.transition.end();
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

	if (!context.transition.is(graphics::TransitionState::inactive)) { return; }

	switch (progress) {
	case 0:
		if (!context.console) { willett->force_engage(); }
		++progress;
		return;
	case 1:
		if (!context.console) {
			willett->flush_conversations();
			willett->push_conversation(11);
			bryn->force_engage();
			player.turn();
			svc.camera_controller.set_position(bryn->Mobile::get_global_center());
			++progress;
			return;
		}
		break;
	case 2:
		if (!context.console) {
			bryn->flush_conversations();
			bryn->push_conversation(11);
			willett->force_engage();
			player.turn();
			svc.camera_controller.set_position(willett->Mobile::get_global_center());
			++progress;
			return;
		}
		break;
	case 3:
		if (!context.console) {
			willett->flush_conversations();
			willett->push_conversation(12);
			bryn->force_engage();
			player.turn();
			svc.camera_controller.set_position(bryn->Mobile::get_global_center());
			++progress;
			return;
		}
		break;
	case 4:
		if (!context.console) {
			bryn->flush_conversations();
			bryn->push_conversation(12);
			willett->force_engage();
			player.turn();
			svc.camera_controller.set_position(willett->Mobile::get_global_center());
			++progress;
			return;
		}
		break;
	case 5:
		if (!context.console) {
			willett->flush_conversations();
			willett->push_conversation(14);
			bryn->force_engage();
			player.turn();
			svc.camera_controller.set_position(bryn->Mobile::get_global_center());
			++progress;
			return;
		}
		break;
	case 6:
		if (!context.console) {
			bryn->pop_conversation();
			willett->force_engage();
			player.turn();
			svc.camera_controller.set_position(willett->Mobile::get_global_center());
			++progress;
			return;
		}
		break;
	case 7:
		if (!context.console && !m_flags.test(PioneerBaseDebriefFlags::end)) {
			cooldowns.end.start();
			cooldowns.pause.start();
			bryn->flush_conversations();
			willett->flush_conversations();
			bryn->push_conversation(6);
			willett->push_conversation(13);
			m_flags.set(PioneerBaseDebriefFlags::end);
			svc.quest_table.set_quest_progression("npc_dialogue", {"bryn", 300}, 1, {3001});
			svc.quest_table.progress_quest("npc_dialogue", {"dr_willett", 300}, 1, -1);
			return;
		}
		break;
	}
}

} // namespace fornani
