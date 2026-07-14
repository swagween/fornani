
#include <fornani/automa/SceneContext.hpp>
#include <fornani/entities/player/Player.hpp>
#include <fornani/gui/console/Console.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/story/cutscene/EncounterLynx.hpp>
#include <fornani/world/Map.hpp>

namespace fornani {

EncounterLynx::EncounterLynx(automa::ServiceProvider& svc) : Cutscene(svc, 227, "encounter_lynx") {
	cooldowns.beginning.set_and_start(40);
	auto prog = svc.quest_table.get_quest_progression("defeat_lynx");
	progress = prog == 0 ? 0 : prog == 1 ? 10 : 20;
	if (progress > 10 || progress == 0) { svc.music_player.stop(); }
}

void EncounterLynx::update(automa::ServiceProvider& svc, SceneContext& context, world::Map& map, player::Player& player) {
	if (complete()) {
		Cutscene::end(svc, player);
		return;
	}

	Cutscene::update(svc, context, map, player);

	if (cooldowns.end.running()) { return; }

	auto npcs = map.get_entities<NPC>();
	auto it = std::ranges::find_if(npcs, [](auto& n) { return n->get_specifier() == 21; });
	auto& lynx = *it;

	auto lynx_enemy = map.get_enemy(15);

	if (cooldowns.end.running()) { lynx->disengage(); }
	if (context.console) { lynx->disengage(); }

	auto enemy_pos = lynx_enemy != nullptr ? lynx_enemy->get_collider().get_center() : player.get_collider().get_center();
	auto camera_focus = progress < 10 ? (player.get_collider().get_center() + lynx->get_collider().get_center()) * 0.5f : enemy_pos;

	auto prog = svc.quest_table.get_quest_progression("lynx_dialogue");
	auto which = prog == 0 ? 1 : 4;

	switch (progress) {
	case 0:
		if (cooldowns.beginning.is_almost_complete()) {
			lynx->unhide();
			lynx->set_invisible();
			lynx->flush_and_push(which);
			lynx->force_engage();
			svc.music_player.stop();
			++progress;
		}
		break;
	case 1:
		if (!context.console) {
			lynx->hide();
			cooldowns.end.set_and_start(4);
			svc.music_player.stop();
			svc.music_player.load(svc.finder, "tumult");
			svc.music_player.play_looped();
			++progress;
		}
		break;
	case 2:
		if (!context.console) {}
		break;
	case 10:
		if (!context.console) {
			lynx->unhide();
			lynx->set_invisible();
			lynx->flush_and_push(3);
			lynx->force_engage();
			++progress;
		}
		break;
	case 11:
		if (!context.console) {
			lynx->hide();
			cooldowns.end.set_and_start(4);
			if (lynx_enemy != nullptr) { lynx_enemy->set_special_event(); }
			svc.music_player.load(svc.finder, "tumultuous_spirit");
			svc.music_player.play_looped();
			++progress;
		}
		break;
	case 20:
		if (cooldowns.beginning.is_almost_complete()) {
			lynx->unhide();
			lynx->set_invisible();
			lynx->flush_and_push(2);
			lynx->force_engage();
			++progress;
		}
		break;
	case 21:
		if (!context.console) {
			lynx->hide();
			if (lynx_enemy != nullptr) { lynx_enemy->set_special_event(); }
			cooldowns.end.set_and_start(4);
			svc.music_player.load(svc.finder, "none");
			svc.music_player.play_looped();
			++progress;
		}
		break;
	}

	svc.camera_controller.set_owner(graphics::CameraOwner::system);
	svc.camera_controller.set_position(camera_focus);
}

} // namespace fornani
