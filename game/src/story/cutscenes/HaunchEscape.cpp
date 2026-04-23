
#include <fornani/automa/SceneContext.hpp>
#include <fornani/entities/player/Player.hpp>
#include <fornani/gui/console/Console.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/story/cutscene/HaunchEscape.hpp>
#include <fornani/world/Map.hpp>

namespace fornani {

HaunchEscape::HaunchEscape(automa::ServiceProvider& svc) : Cutscene(svc, 902, "haunch_escape"), m_intro{200}, m_dynamite{10}, m_bomb_tick{300}, m_heroes_exit{600}, m_outro{500}, m_champion_entry{600} {
	m_intro.start();
	svc.music_player.load(svc.finder, "haunchs_theme");
	svc.input_system.flush_inputs();
	svc.state_flags.set(automa::StateFlags::cutscene);
}

void HaunchEscape::update(automa::ServiceProvider& svc, SceneContext& context, world::Map& map, player::Player& player) {
	if (m_flags.consume(HaunchEscapeFlags::done)) {
		player.controller.unrestrict();
		svc.state_flags.reset(automa::StateFlags::hide_hud);
		svc.state_flags.reset(automa::StateFlags::no_menu);
		svc.state_flags.reset(automa::StateFlags::cutscene);
		svc.camera_controller.set_owner(graphics::CameraOwner::player);
		svc.music_player.stop();
		player.set_flag(player::PlayerFlags::cutscene, false);
		svc.camera_controller.constrain();
		m_flags.set(HaunchEscapeFlags::over);
		return;
	}
	if (player.is_dead()) { return; }

	svc.state_flags.set(automa::StateFlags::no_menu);
	cooldowns.beginning.update();
	cooldowns.pause.update();
	cooldowns.long_pause.update();
	cooldowns.end.update();
	m_heroes_exit.update();
	m_outro.update();
	m_champion_entry.update();
	if (!context.console) { m_bomb_tick.update(); }

	if (m_champion) { m_champion->update(map); }

	auto evade_sequence = progress == 2 || progress == 3;

	auto npcs = map.get_entities<NPC>();
	auto hit = std::ranges::find_if(npcs, [](auto& n) { return n->get_specifier() == 39; });
	auto& haunch = *hit;
	auto bit = std::ranges::find_if(npcs, [](auto& n) { return n->get_specifier() == 34; });
	auto& bryn = *bit;

	auto haunch_enemy = map.get_enemy(28);

	if (!evade_sequence) {
		svc.state_flags.set(automa::StateFlags::cutscene);
		svc.state_flags.set(automa::StateFlags::hide_hud);
		player.controller.restrict_movement();
		player.stall_idle_timer();
		player.set_flag(player::PlayerFlags::show_weapon, false);

		svc.camera_controller.set_owner(graphics::CameraOwner::system);
		svc.camera_controller.free();

		if (haunch_enemy != nullptr) {
			svc.camera_controller.set_position(haunch_enemy->get_global_center());
			// haunch->set_position(haunch_enemy->get_global_center());
		}
	}

	if (m_intro.just_started()) {
		if (context.console) { context.console.reset(); }
		player.controller.prevent_movement();
		player.get_collider().physics.zero_x();
		player.set_flag(player::PlayerFlags::cutscene);
		player.set_idle();
		haunch->set_flag(NPCFlags::cutscene);
		haunch->flush_and_push(3);
		m_champion.emplace(svc, map);
	}

	if (m_intro.is_almost_complete()) { cooldowns.beginning.start(); }
	m_intro.update();
	if (m_intro.running()) { return; }

	if (context.console) { context.console.value()->set_no_exit(true); }

	auto total_suites{0};
	for (auto& npc : npcs) { total_suites += npc->get_number_of_suites(); }
	total_conversations = std::max(total_conversations, total_suites);
	if (cooldowns.end.is_almost_complete()) {
		flags.set(CutsceneFlags::complete);
		return;
	}
	if (npcs.empty()) { return; }

	if (cooldowns.end.running()) { haunch->disengage(); }
	if (cooldowns.beginning.is_almost_complete()) {}
	if (context.console) { haunch->disengage(); }
	if (context.console) { bryn->disengage(); }

	if (m_bomb_tick.is_almost_complete()) {
		m_dynamite.update();
		m_bomb_tick.start();
		svc.soundboard.play_sound("bomb_countdown");
	}

	if (m_champion) { bryn->set_position(m_champion->get_global_center() + sf::Vector2f{-4.f, -4.f}); }
	bryn->request(NPCAnimationState::special_2);

	switch (progress) {
	case 0:
		haunch->force_engage();
		++progress;
		break;
	case 1:
		if (!context.console) {
			m_dynamite.start();
			m_bomb_tick.start();
			if (haunch_enemy != nullptr) { haunch_enemy->set_special_event(); }
			player.controller.unrestrict();
			svc.state_flags.reset(automa::StateFlags::hide_hud);
			svc.state_flags.reset(automa::StateFlags::cutscene);
			svc.camera_controller.set_owner(graphics::CameraOwner::player);
			player.set_flag(player::PlayerFlags::cutscene, false);
			svc.camera_controller.constrain();
			++progress;
		}
		break;
	case 2:
		if (m_dynamite.get() == 8 && !context.console) {
			haunch->flush_and_push(4);
			haunch->force_engage();
			++progress;
		}
		break;
	case 3:
		if (m_dynamite.get() == 4 && !context.console) { ++progress; }
		break;
	case 4:
		// bryn rescues nani
		if (!context.console) {
			bryn->unhide();
			bryn->set_invisible(false);
			player.controller.prevent_movement();
			player.get_collider().physics.zero_x();
			player.set_flag(player::PlayerFlags::cutscene);
			player.set_idle();
			if (m_champion) { m_champion->set_target(player.get_collider().get_top() - sf::Vector2f{0.f, 40.f}); }
			m_champion_entry.start();
			++progress;
		}
		break;
	case 5:
		if (m_champion_entry.is_almost_complete() && m_champion) { /*m_champion->flags.set(ChampionJ5Flags::interactable);*/
		}
		if (!context.console.has_value()) {
			if (bryn->get_collider().bounding_box.overlaps(player.get_collider().get_vicinity_rect())) {
				bryn->flush_and_push(4);
				bryn->force_engage();
				++progress;
			}
		}
		break;
	case 6: break;
	case 8:
		if (!context.console) {
			if (m_champion) {
				m_champion->request(ChampionJ5State::take_off);
				m_champion->set_target(sf::Vector2f{3000.f, -100.f});
				player.set_position(m_champion->get_global_center());
			}
			player.request_animation(player::AnimState::sit);
			++progress;
			m_heroes_exit.start();
		}
		break;
	case 9:
		if (m_champion) { player.set_position(m_champion->get_global_center()); }
		if (m_heroes_exit.is_almost_complete()) {
			haunch->flush_and_push(5);
			haunch->force_engage();
			++progress;
		}
		break;
	case 10: break;
	case 11:
		svc.data.switch_destructible_state(90102, true);
		m_outro.start();
		++progress;
		break;
	case 12:
		if (!context.console) {
			if (svc.ticker.every_x_ticks(80)) {
				auto pos = random::random_vector_float(-300.f, 300.f);
				map.spawn_explosion(svc, "explosion", "explosion", arms::Team::skycorps, pos, 48.f, 1, 4);
			}
			if (m_outro.is_almost_complete()) { cooldowns.end.start(); }
		}
		break;
	default: break;
	}
}

void HaunchEscape::render(sf::RenderWindow& win, sf::Vector2f cam) {
	if (m_champion) { m_champion->render(win, cam); }
}

} // namespace fornani
