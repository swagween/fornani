
#include <fornani/automa/SceneContext.hpp>
#include <fornani/entities/player/Player.hpp>
#include <fornani/gui/console/Console.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/story/cutscene/MomsGrave.hpp>
#include <fornani/world/Map.hpp>

namespace fornani {

MomsGrave::MomsGrave(automa::ServiceProvider& svc) : Cutscene(svc, 402, "moms_grave") {
	cooldowns.beginning.set_and_start(400);
	svc.music_player.stop();
}

void MomsGrave::update(automa::ServiceProvider& svc, SceneContext& context, world::Map& map, player::Player& player) {
	if (complete()) {
		Cutscene::end(svc, player);
		svc.music_player.resume();
		svc.quest_table.set_quest_progression("moms_grave", 1);
		return;
	}

	Cutscene::update(svc, context, map, player);

	if (cooldowns.beginning.just_started()) {
		player.controller.prevent_movement();
		player.get_collider().physics.zero_x();
		if (player.get_actual_direction().left()) { player.set_direction(Direction{LR::right}); }
		player.set_slow_walk();
	}

	switch (progress) {
	case 0: {
		if (cooldowns.beginning.is_almost_complete()) {
			player.set_idle();
			cooldowns.long_pause.start();
			player.set_sitting();
			++progress;
		}
		break;
	}
	case 1: {
		if (cooldowns.long_pause.is_almost_complete()) {
			context.console.emplace(std::make_unique<gui::Console>(svc, svc.text.story, "moms_grave", gui::OutputType::no_skip));
			++progress;
		}
		break;
	}
	case 2: {
		if (!context.console) {
			cooldowns.end.start();
			++progress;
		}
		break;
	}
	}

	svc.camera_controller.set_owner(graphics::CameraOwner::system);
	svc.camera_controller.set_position(player.get_camera_focus_point());
}

} // namespace fornani
