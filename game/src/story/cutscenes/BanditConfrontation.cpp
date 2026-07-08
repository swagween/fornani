
#include <fornani/automa/SceneContext.hpp>
#include <fornani/entities/player/Player.hpp>
#include <fornani/gui/console/Console.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/story/cutscene/BanditConfrontation.hpp>
#include <fornani/world/Map.hpp>

namespace fornani {

BanditConfrontation::BanditConfrontation(automa::ServiceProvider& svc) : Cutscene(svc, 1301, "bandit_confrontation"), m_bandit_escape{260}, m_threaten{260} {
	cooldowns.beginning.start();
	svc.music_player.stop();
}

void BanditConfrontation::update(automa::ServiceProvider& svc, SceneContext& context, world::Map& map, player::Player& player) {
	if (complete()) {
		player.controller.unrestrict();
		svc.state_flags.reset(automa::StateFlags::hide_hud);
		svc.state_flags.reset(automa::StateFlags::no_menu);
		svc.state_flags.reset(automa::StateFlags::cutscene);
		svc.camera_controller.set_owner(graphics::CameraOwner::player);
		svc.quest_table.set_quest_progression("ashtown_bandit", 10);
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
	m_bandit_escape.update();
	m_threaten.update();

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
		bandit->set_special_animation(3);
		map.spawn_effect(svc, "exclamation_mark", bandit->get_collider().physics.position + sf::Vector2f{-12.f, -12.f});
		svc.soundboard.play_sound("alert_bell");
		++progress;
		break;
	case 2: bandit->request(NPCAnimationState::idle); break;
	case 3:
		if (!context.console) {
			m_threaten.start();
			++progress;
		}
		break;
	case 4:
		if (m_threaten.get() == 80 && player.arsenal && player.hotbar) {
			player.equipped_weapon().shoot(svc, map);
			bandit->set_special_animation(3);
			map.spawn_effect(svc, "exclamation_mark", bandit->get_collider().physics.position + sf::Vector2f{-12.f, -12.f});
			svc.soundboard.play_sound("alert_bell");
			svc.soundboard.play_sound("hit_squeak");
		}
		if (m_threaten.is_complete()) {
			bandit->flush_and_push(3);
			bandit->force_engage();
			++progress;
		}
		break;
	case 5:
		bandit->request(NPCAnimationState::idle);
		if (!context.console) {
			bandit->set_special_animation(2);
			m_bandit_escape.start();
			++progress;
		}
		break;
	case 6:
		if (m_bandit_escape.is_almost_complete()) {
			map.spawn_effect(svc, "explosion", bandit->get_collider().get_center(), {}, 1);
			svc.soundboard.play_sound("disappear");
			bandit->hide();
			cooldowns.end.start(400);
			++progress;
		}
		break;
	}

	svc.camera_controller.set_owner(graphics::CameraOwner::system);
	svc.camera_controller.set_position(camera_focus);
}

} // namespace fornani
