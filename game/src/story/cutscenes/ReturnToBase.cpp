
#include <fornani/automa/SceneContext.hpp>
#include <fornani/entities/player/Player.hpp>
#include <fornani/gui/console/Console.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/story/cutscene/ReturnToBase.hpp>
#include <fornani/world/Map.hpp>

namespace fornani {

ReturnToBase::ReturnToBase(automa::ServiceProvider& svc) : Cutscene(svc, 209, "return_to_base"), m_intro{200} {
	m_intro.start();
	svc.input_system.flush_inputs();
}

void ReturnToBase::update(automa::ServiceProvider& svc, SceneContext& context, world::Map& map, player::Player& player) {
	if (complete()) {
		flags.set(CutsceneFlags::delete_me);
		svc.quest_table.set_quest_progression("defeat_haunch", 3);
	}

	svc.state_flags.set(automa::StateFlags::no_menu);
	cooldowns.beginning.update();
	cooldowns.pause.update();
	cooldowns.long_pause.update();
	cooldowns.end.update();

	if (m_champion) {
		if (m_champion->is_close_to_target(2.f)) { m_champion->flags.set(ChampionJ5Flags::interactable); }
		m_champion->update(svc, map);
	}

	auto npcs = map.get_entities<NPC>();
	auto bit = std::ranges::find_if(npcs, [](auto& n) { return n->get_specifier() == 34; });
	auto& bryn = *bit;

	svc.state_flags.set(automa::StateFlags::cutscene);
	svc.state_flags.set(automa::StateFlags::hide_hud);
	player.controller.restrict_movement();
	player.stall_idle_timer();
	player.set_flag(player::PlayerFlags::show_weapon, false);

	svc.camera_controller.set_owner(graphics::CameraOwner::system);
	svc.camera_controller.free();

	svc.camera_controller.set_position(player.get_camera_focus_point());

	if (m_intro.just_started()) {
		if (context.console) { context.console.reset(); }
		player.controller.prevent_movement();
		player.get_collider().physics.zero_x();
		player.set_flag(player::PlayerFlags::cutscene);
		player.set_sitting();
		player.set_direction({LR::right});
		m_champion.emplace(svc, map);
		m_champion->get_collider().set_position(sf::Vector2f{200.f, 100.f});
		m_champion->set_target(constants::f_cell_size * sf::Vector2f{40.f, 8.f});
	}

	if (m_intro.is_almost_complete()) { cooldowns.beginning.start(); }
	m_intro.update();

	if (context.console) { context.console.value()->set_no_exit(true); }

	auto total_suites{0};
	for (auto& npc : npcs) { total_suites += npc->get_number_of_suites(); }
	total_conversations = std::max(total_conversations, total_suites);
	if (cooldowns.end.is_almost_complete()) {
		flags.set(CutsceneFlags::complete);
		return;
	}
	if (npcs.empty()) { return; }

	if (cooldowns.beginning.is_almost_complete()) {}
	if (context.console) { bryn->disengage(); }
	bryn->set_flag(NPCFlags::cutscene);

	if (m_champion) { bryn->set_position(m_champion->get_drivers_seat()); }
	bryn->request(NPCAnimationState::special_2);

	auto champion_target = player.get_collider().get_top() - sf::Vector2f{0.f, 40.f};
	auto player_seat = player.get_collider().get_top();
	if (m_champion) { player.set_position(m_champion->get_passengers_seat()); }
	if (m_intro.running()) { return; }

	switch (progress) {
	case 0:
		if (m_intro.is_complete()) {
			bryn->flush_and_push(7);
			bryn->force_engage();
		}
		break;
	default: break;
	}
}

void ReturnToBase::render(sf::RenderWindow& win, sf::Vector2f cam) {
	if (m_champion) { m_champion->render(win, cam); }
}

} // namespace fornani
