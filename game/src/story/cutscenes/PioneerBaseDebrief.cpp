
#include <fornani/entities/player/Player.hpp>
#include <fornani/gui/console/Console.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/story/cutscene/PioneerBaseDebrief.hpp>
#include <fornani/world/Map.hpp>

namespace fornani {

PioneerBaseDebrief::PioneerBaseDebrief(automa::ServiceProvider& svc) : Cutscene(svc, 300, "pioneer_base_debrief") { cooldowns.beginning.start(); }

void PioneerBaseDebrief::update(automa::ServiceProvider& svc, std::optional<std::unique_ptr<gui::Console>>& console, world::Map& map, player::Player& player) {

	if (complete() && map.transition.is(graphics::TransitionState::inactive)) {
		map.transition.start();
		return;
	} else if (complete() && map.transition.is(graphics::TransitionState::black)) {
		player.controller.unrestrict();
		svc.state_flags.reset(automa::StateFlags::hide_hud);
		svc.state_flags.reset(automa::StateFlags::no_menu);
		svc.state_flags.reset(automa::StateFlags::cutscene);
		svc.camera_controller.set_owner(graphics::CameraOwner::player);
		svc.quest_table.progress_quest("defeat_skycorps", 1, 50901);
		return;
	}

	if (map.transition.is(graphics::TransitionState::black)) { map.transition.end(); }

	static auto progress = util::Counter{};

	svc.state_flags.set(automa::StateFlags::hide_hud);
	svc.state_flags.set(automa::StateFlags::no_menu);
	svc.state_flags.set(automa::StateFlags::cutscene);
	cooldowns.beginning.update();
	cooldowns.pause.update();
	cooldowns.long_pause.update();
	cooldowns.end.update();

	player.controller.restrict_movement();

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
		bryn->push_conversation(10);
		willett->push_conversation(10);
		flags.set(CutsceneFlags::started);
	}

	auto total_suites{0};
	for (auto& npc : npcs) { total_suites += npc->get_number_of_suites(); }
	total_conversations = std::max(total_conversations, total_suites);
	if (cooldowns.end.is_almost_complete()) {
		flags.set(CutsceneFlags::complete);
		return;
	}
	if (cooldowns.beginning.is_almost_complete()) { map.transition.start(); }
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
	svc.camera_controller.set_position(willett->Mobile::get_global_center());

	if (!map.transition.is(graphics::TransitionState::inactive)) { return; }

	switch (progress.get_count()) {
	case 0:
		if (!console) { willett->force_engage(); }
		progress.update();
		return;
	case 1:
		if (!console) {
			willett->pop_conversation();
			bryn->force_engage();
			progress.update();
			return;
		}
		break;
	case 2: break;
	}
}

} // namespace fornani
