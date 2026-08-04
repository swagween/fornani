
#include <fornani/automa/SceneContext.hpp>
#include <fornani/entities/player/Player.hpp>
#include <fornani/gui/console/Console.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/story/cutscene/BitCell.hpp>
#include <fornani/world/Map.hpp>

namespace fornani {

BitCell::BitCell(automa::ServiceProvider& svc) : Cutscene(svc, 101, "bit_cell") {
	cooldowns.beginning.start();
	svc.music_player.stop();
}

void BitCell::update(automa::ServiceProvider& svc, SceneContext& context, world::Map& map, player::Player& player) {
	if (complete()) {
		svc.quest_table.progress_quest("free_bit", 1, 101);
		svc.music_player.resume();
		Cutscene::end(svc, player);
		return;
	}

	svc.state_flags.set(automa::StateFlags::hide_hud);
	svc.state_flags.set(automa::StateFlags::no_menu);
	svc.state_flags.set(automa::StateFlags::cutscene);
	svc.camera_controller.free();
	player.stall_idle_timer();
	cooldowns.beginning.update();
	cooldowns.pause.update();
	cooldowns.long_pause.update();
	cooldowns.end.update();

	player.controller.restrict_movement();

	if (context.console) { context.console.value()->set_no_exit(true); }

	auto npcs = map.get_entities<NPC>();
	auto it = std::ranges::find_if(npcs, [](auto& n) { return n->get_specifier() == 20; });
	auto& bit = *it;

	if (cooldowns.end.is_almost_complete()) {
		flags.set(CutsceneFlags::complete);
		return;
	}

	if (cooldowns.end.running()) { bit->disengage(); }
	if (cooldowns.beginning.is_almost_complete()) {
		if (!context.console.has_value()) {
			bit->set_flag(NPCFlags::face_player);
			bit->force_engage();
		}
	}
	if (context.console) { bit->disengage(); }

	auto camera_focus = (player.get_collider().get_center() + bit->get_collider().get_center()) * 0.5f;

	switch (progress) {
	case 1:
		if (!context.console) {
			bit->get_collider().physics.velocity.x = 2.f;
			bit->set_direction({LR::right});
			bit->face_movement();
			bit->walk();
			bit->set_flag(NPCFlags::face_player, false);
			bit->set_flag(NPCFlags::cutscene);
			for (auto p : map.get_entities<Portal>()) {
				if (std::abs(bit->get_collider().get_center().x - p->bounding_box.get_center().x) < 4.f) {
					bit->set_flag(NPCFlags::face_player);
					bit->flush_conversations();
					bit->push_conversation(5);
					bit->force_engage();
					player.turn();
					bit->set_flag(NPCFlags::cutscene, false);
					++progress;
				}
			}
		}
		break;
	case 2:
		if (!context.console) {
			bit->use_portal(map);
			if (bit->is_hidden()) {
				cooldowns.end.start();
				++progress;
			}
		}
		break;
	}

	svc.camera_controller.set_owner(graphics::CameraOwner::system);
	svc.camera_controller.set_position(camera_focus);
}

} // namespace fornani
