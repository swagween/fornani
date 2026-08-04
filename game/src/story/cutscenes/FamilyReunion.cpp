
#include <fornani/automa/SceneContext.hpp>
#include <fornani/entities/player/Player.hpp>
#include <fornani/gui/console/Console.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/story/cutscene/FamilyReunion.hpp>
#include <fornani/world/Map.hpp>

namespace fornani {

FamilyReunion::FamilyReunion(automa::ServiceProvider& svc) : Cutscene(svc, 407, "family_reunion") {
	cooldowns.beginning.start();
	svc.music_player.stop();
}

void FamilyReunion::update(automa::ServiceProvider& svc, SceneContext& context, world::Map& map, player::Player& player) {
	if (complete()) {
		Cutscene::end(svc, player);
		svc.quest_table.set_quest_progression("reunite_with_dad", 1);
		return;
	}

	Cutscene::update(svc, context, map, player);

	auto npcs = map.get_entities<NPC>();
	auto it = std::ranges::find_if(npcs, [](auto& n) { return n->get_specifier() == 8; });
	auto wit = std::ranges::find_if(npcs, [](auto& n) { return n->get_specifier() == 27; });
	auto& jimmy = *it;
	auto& wames = *wit;
	if (context.console) { jimmy->disengage(); }
	if (context.console) { wames->disengage(); }
	jimmy->set_flag(NPCFlags::cutscene);
	wames->set_flag(NPCFlags::cutscene);
	jimmy->set_flag(NPCFlags::custom_camera);

	auto camera_focus = (player.get_collider().get_center() + jimmy->get_collider().get_center()) * 0.5f;

	switch (progress) {
	case 0: {
		cooldowns.pause.start(450);
		++progress;
		break;
	}
	case 1: {
		jimmy->walk();
		if (cooldowns.pause.is_almost_complete()) {
			svc.music_player.resume();
			jimmy->request(NPCAnimationState::idle);
			jimmy->flush_and_push(1);
			jimmy->force_engage();
			++progress;
		}
		break;
	}
	case 2: {
		if (!context.console) {
			cooldowns.pause.start();
			++progress;
		}
		break;
	}
	case 3: {
		if (cooldowns.pause.is_almost_complete()) {
			wames->flush_and_push(2);
			wames->force_engage();
			++progress;
		}
		break;
	}
	case 4: {
		if (!context.console) {
			cooldowns.pause.start();
			++progress;
		}
		break;
	}
	case 5: {
		if (cooldowns.pause.is_almost_complete()) {
			jimmy->flush_and_push(3);
			jimmy->force_engage();
			++progress;
		}
		break;
	}
	case 10: {
		cooldowns.end.start();
		++progress;
		jimmy->flush_and_push(2);
		break;
	}
	}

	svc.camera_controller.set_owner(graphics::CameraOwner::system);
	svc.camera_controller.set_position(camera_focus);
}

} // namespace fornani
