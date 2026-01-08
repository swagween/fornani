
#include <fornani/entities/player/Player.hpp>
#include <fornani/gui/console/Console.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/story/cutscene/BrynPostMiaag.hpp>
#include <fornani/world/Map.hpp>

namespace fornani {

BrynPostMiaag::BrynPostMiaag(automa::ServiceProvider& svc) : Cutscene(svc, 509, "bryn_post_miaag") { cooldowns.beginning.start(); }

void BrynPostMiaag::update(automa::ServiceProvider& svc, std::optional<std::unique_ptr<gui::Console>>& console, world::Map& map, player::Player& player) {

	if (complete()) {
		player.controller.unrestrict();
		svc.state_flags.reset(automa::StateFlags::hide_hud);
		svc.state_flags.reset(automa::StateFlags::no_menu);
		svc.state_flags.reset(automa::StateFlags::cutscene);
		svc.camera_controller.set_owner(graphics::CameraOwner::player);
		svc.quest_table.progress_quest("defeat_miaag", 1, 50901);
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

	if (console) { console.value()->set_no_exit(true); }

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
	if (cooldowns.beginning.is_almost_complete()) {
		map.reveal_npc("bryn");
		NANI_LOG_DEBUG(p_logger, "Bryn was revealed.");
	}
	if (npcs.empty()) { return; }
	if (cooldowns.beginning.running()) {
		player.controller.prevent_movement();
		player.controller.set_direction(Direction{UND::neutral, LNR::left});
		return;
	}

	// get npcs
	if (cooldowns.end.running()) { bryn->disengage(); }
	if (bryn->get_collider().bounding_box.overlaps(player.get_collider().get_vicinity_rect())) {
		if (!console) {
			bryn->force_engage();
			bryn->request(NPCAnimationState::idle);
			cooldowns.end.start(4);
		}
	} else {
		bryn->walk();
	}
	svc.camera_controller.set_owner(graphics::CameraOwner::system);
	svc.camera_controller.set_position(bryn->Mobile::get_global_center());
}

} // namespace fornani
