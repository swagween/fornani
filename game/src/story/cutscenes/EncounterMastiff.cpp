
#include <fornani/automa/SceneContext.hpp>
#include <fornani/entities/player/Player.hpp>
#include <fornani/gui/console/Console.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/story/cutscene/EncounterMastiff.hpp>
#include <fornani/world/Map.hpp>

namespace fornani {

EncounterMastiff::EncounterMastiff(automa::ServiceProvider& svc) : Cutscene(svc, 401, "encounter_mastiff") {
	cooldowns.beginning.set_and_start(400);
	cooldowns.long_pause.start();
	svc.music_player.load(svc.finder, "none");
}

void EncounterMastiff::update(automa::ServiceProvider& svc, SceneContext& context, world::Map& map, player::Player& player) {
	if (complete()) {
		svc.events.start_battle_event.dispatch();
		Cutscene::end(svc, player);
		return;
	}

	Cutscene::update(svc, context, map, player);
	svc.state_flags.reset(automa::StateFlags::cutscene);
	svc.camera_controller.constrain();

	auto mastiff = map.get_enemy(24);

	auto enemy_pos = mastiff != nullptr ? mastiff->get_collider().get_center() : player.get_collider().get_center();
	auto camera_focus = progress < 1 ? player.get_camera_focus_point() : (player.get_collider().get_center() + enemy_pos) * 0.5f;

	switch (progress) {
	case 0:
		if (cooldowns.long_pause.is_almost_complete()) {
			cooldowns.long_pause.start();
			svc.camera_controller.shake();
			svc.soundboard.play_sound("vibration");
			++progress;
		}
		break;
	case 1:
		if (cooldowns.long_pause.is_almost_complete()) { ++progress; }
		break;
	case 2:
		mastiff->set_special_event();
		svc.soundboard.play_sound("grand_mastiff_howl");
		svc.soundboard.play_sound("heart_spin");
		++progress;
		break;
	case 3:
		cooldowns.end.start();
		++progress;
		break;
	}
	if (progress > 0) { svc.camera_controller.set_owner(graphics::CameraOwner::system); }
	svc.camera_controller.set_position(camera_focus);
}

} // namespace fornani
