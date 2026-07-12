
#include <fornani/automa/SceneContext.hpp>
#include <fornani/entities/player/Player.hpp>
#include <fornani/gui/console/Console.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/story/cutscene/EncounterMinigus.hpp>
#include <fornani/world/Map.hpp>

namespace fornani {

EncounterMinigus::EncounterMinigus(automa::ServiceProvider& svc) : Cutscene(svc, 117, "encounter_minigus") {
	cooldowns.beginning.set_and_start(40);
	svc.music_player.stop();
	svc.music_player.load(svc.finder, "minigus");
	svc.music_player.play_looped();
	progress = svc.quest_table.get_quest_progression("defeat_minigus") == 0 ? 0 : 10;
}

void EncounterMinigus::update(automa::ServiceProvider& svc, SceneContext& context, world::Map& map, player::Player& player) {
	if (complete()) {
		progress < 10 ? svc.music_player.load(svc.finder, "scuffle") : svc.music_player.load("dusken");
		svc.music_player.play_looped();
		Cutscene::end(svc, player);
		return;
	}

	Cutscene::update(svc, context, map, player);

	auto npcs = map.get_entities<NPC>();
	auto it = std::ranges::find_if(npcs, [](auto& n) { return n->get_specifier() == 7; });
	auto& gus = *it;

	auto gus_enemy = map.get_enemy(6);

	if (cooldowns.end.running()) { gus->disengage(); }
	if (context.console) { gus->disengage(); }

	auto enemy_pos = gus_enemy != nullptr ? gus_enemy->get_collider().get_center() : player.get_collider().get_center();
	auto camera_focus = progress < 10 ? (player.get_collider().get_center() + gus->get_collider().get_center()) * 0.5f : enemy_pos;

	auto prog = svc.quest_table.get_quest_progression("minigus_dialogue");
	auto which = prog == 0 ? 1 : 4;

	switch (progress) {
	case 0:
		if (cooldowns.beginning.is_almost_complete()) {
			gus->unhide();
			gus->set_invisible();
			gus->flush_and_push(which);
			gus->force_engage();
			++progress;
		}
		break;
	case 1:
		if (!context.console) {
			gus->hide();
			cooldowns.end.set_and_start(4);
			++progress;
		}
		break;
	case 2:
		if (!context.console) {}
		break;
	case 10:
		if (!context.console) {
			svc.music_player.pause();
			gus->unhide();
			gus->set_invisible();
			gus->flush_and_push(2);
			gus->force_engage();
			++progress;
		}
		break;
	case 11:
		if (!context.console) {
			cooldowns.long_pause.start();
			++progress;
		}
		break;
	case 12:
		if (cooldowns.long_pause.is_almost_complete()) {
			gus->flush_and_push(3);
			gus->force_engage();
			++progress;
		}
		break;
	case 13:
		if (!context.console) {
			gus->hide();
			if (gus_enemy != nullptr) { gus_enemy->set_special_event(); }
			cooldowns.end.set_and_start(4);
			++progress;
		}
		break;
	}

	svc.camera_controller.set_owner(graphics::CameraOwner::system);
	svc.camera_controller.set_position(camera_focus);
}

} // namespace fornani
