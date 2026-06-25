
#include <fornani/automa/SceneContext.hpp>
#include <fornani/entities/player/Player.hpp>
#include <fornani/gui/console/Console.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/story/cutscene/BanditEncounter.hpp>
#include <fornani/world/Map.hpp>

namespace fornani {

BanditEncounter::BanditEncounter(automa::ServiceProvider& svc) : Cutscene(svc, 1002, "bandit_encounter") {
	cooldowns.beginning.start();
	svc.music_player.stop();
}

void BanditEncounter::update(automa::ServiceProvider& svc, SceneContext& context, world::Map& map, player::Player& player) {
	if (complete()) {
		player.controller.unrestrict();
		svc.state_flags.reset(automa::StateFlags::hide_hud);
		svc.state_flags.reset(automa::StateFlags::no_menu);
		svc.state_flags.reset(automa::StateFlags::cutscene);
		svc.camera_controller.set_owner(graphics::CameraOwner::player);
		svc.quest_table.progress_quest("ashtown_bandit", 1, 1002);
		svc.music_player.resume();
		flags.set(CutsceneFlags::delete_me);
		return;
	}

	svc.state_flags.set(automa::StateFlags::hide_hud);
	svc.state_flags.set(automa::StateFlags::no_menu);
	svc.state_flags.set(automa::StateFlags::cutscene);
	player.stall_idle_timer();
	cooldowns.beginning.update();
	cooldowns.pause.update();
	cooldowns.long_pause.update();
	cooldowns.end.update();

	player.controller.restrict_movement();

	if (context.console) { context.console.value()->set_no_exit(true); }

	auto npcs = map.get_entities<NPC>();
	auto bit = std::ranges::find_if(npcs, [](auto& n) { return n->get_specifier() == 42; });
	auto& bandit = *bit;

	if (cooldowns.end.is_almost_complete()) {
		flags.set(CutsceneFlags::complete);
		return;
	}

	if (cooldowns.end.running()) { bandit->disengage(); }
	if (cooldowns.beginning.is_almost_complete()) {
		if (!context.console.has_value()) { bandit->force_engage(); }
	}
	if (context.console) { bandit->disengage(); }

	auto camera_focus = bandit->get_collider().get_center();

	switch (progress) {
	case 1:
		if (!context.console) {
			bandit->set_special_animation(3);
			map.spawn_effect(svc, "exclamation_mark", bandit->get_collider().physics.position + sf::Vector2f{-12.f, -12.f});
			++progress;
		}
		break;
	case 2:
		if (bandit->is_animation_complete()) { ++progress; }
		break;
	case 3:
		bandit->get_collider().physics.velocity.x = 2.f;
		bandit->set_direction({LR::right});
		bandit->face_movement();
		bandit->walk();
		bandit->use_portal(map);
		if (bandit->is_hidden()) {
			cooldowns.end.start();
			++progress;
		}
		break;
	}

	svc.camera_controller.set_owner(graphics::CameraOwner::system);
	svc.camera_controller.set_position(camera_focus);
}

} // namespace fornani
