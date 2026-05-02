
#include <fornani/automa/SceneContext.hpp>
#include <fornani/entities/player/Player.hpp>
#include <fornani/gui/console/Console.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/story/cutscene/ReturnToBase.hpp>
#include <fornani/world/Map.hpp>

namespace fornani {

ReturnToBase::ReturnToBase(automa::ServiceProvider& svc) : Cutscene(svc, 209, "return_to_base"), m_intro{200}, m_landed{200} {
	m_intro.start();
	svc.input_system.flush_inputs();
	svc.camera_controller.set_owner(graphics::CameraOwner::player);
	svc.camera_controller.constrain();
}

void ReturnToBase::update(automa::ServiceProvider& svc, SceneContext& context, world::Map& map, player::Player& player) {

	auto npcs = map.get_entities<NPC>();
	auto bit = std::ranges::find_if(npcs, [](auto& n) { return n->get_specifier() == 34; });
	auto& bryn = *bit;

	if (complete() && !m_flags.test(ReturnToBaseFlags::done) && context.transition.is_black()) {
		m_flags.set(ReturnToBaseFlags::done);
		player.controller.unrestrict();
		svc.state_flags.reset(automa::StateFlags::hide_hud);
		svc.state_flags.reset(automa::StateFlags::no_menu);
		svc.state_flags.reset(automa::StateFlags::cutscene);
		player.set_flag(player::PlayerFlags::cutscene, false);
		svc.quest_table.progress_quest("defeat_skycorps", 1, 999);
		svc.camera_controller.set_owner(graphics::CameraOwner::player);
		svc.camera_controller.constrain();
		context.transition.end();
		bryn->hide();
		svc.quest_table.set_quest_progression("npc_dialogue", {"dr_willett", 300}, 4, {209});
		svc.quest_table.set_quest_progression("npc_dialogue", {"bryn", 300}, 2, {209});
	}

	if (m_flags.test(ReturnToBaseFlags::done)) { return; }

	svc.state_flags.set(automa::StateFlags::no_menu);
	cooldowns.beginning.update();
	cooldowns.pause.update();
	cooldowns.long_pause.update();
	cooldowns.end.update();
	m_landed.update();

	if (m_champion) {
		if (m_champion->is_close_to_target(2.f)) { m_champion->flags.set(ChampionJ5Flags::interactable); }
		m_champion->update(svc, map);
	}

	svc.state_flags.set(automa::StateFlags::hide_hud);
	player.controller.restrict_movement();
	player.stall_idle_timer();
	player.set_flag(player::PlayerFlags::show_weapon, false);

	if (m_intro.just_started()) {
		if (context.console) { context.console.reset(); }
		player.controller.prevent_movement();
		player.get_collider().physics.zero_x();
		player.set_flag(player::PlayerFlags::cutscene);
		player.set_sitting();
		player.set_direction({LR::right});
		m_champion.emplace(svc, map);
		m_champion->set_channel(1);
		m_champion->get_collider().set_position(sf::Vector2f{200.f, 100.f});
		m_champion->set_target(constants::f_cell_size * sf::Vector2f{40.f, 8.f});
		bryn->set_flag(NPCFlags::airborne);
		bryn->set_flag(NPCFlags::custom_camera);
	}

	if (m_intro.is_almost_complete()) { cooldowns.beginning.start(); }
	m_intro.update();

	if (context.console) { context.console.value()->set_no_exit(true); }

	auto total_suites{0};
	for (auto& npc : npcs) { total_suites += npc->get_number_of_suites(); }
	total_conversations = std::max(total_conversations, total_suites);
	if (npcs.empty()) { return; }

	auto going = progress < 5;
	if (context.console) { bryn->disengage(); }
	if (going) {
		bryn->set_special_animation(2);
		bryn->set_flag(NPCFlags::cutscene);
	}
	if (m_champion && going) { bryn->set_position(m_champion->get_drivers_seat()); }
	if (m_champion && going) { player.set_position(m_champion->get_passengers_seat()); }

	if (m_intro.running()) { return; }

	switch (progress) {
	case 0:
		if (m_intro.is_complete()) {
			bryn->flush_and_push(7);
			bryn->force_engage();
			++progress;
		}
		break;
	case 1:
		if (m_champion) {
			if (m_champion->flags.test(ChampionJ5Flags::landed) && !m_landed.running()) { m_landed.start(); }
			if (m_landed.is_almost_complete()) {
				bryn->flush_and_push(8);
				bryn->force_engage();
				bryn->set_special_animation(3);
				bryn->set_flag(NPCFlags::face_player);
				bryn->set_flag(NPCFlags::airborne, false);
				bryn->set_flag(NPCFlags::custom_camera, false);
				player.set_idle();
				++progress;
			}
		}
		break;
	case 4:
		if (!context.console) {
			bryn->flush_and_push(9);
			flags.set(CutsceneFlags::complete);
			context.transition.start();
			svc.quest_table.set_quest_progression("defeat_haunch", 3);
			bryn->set_flag(NPCFlags::cutscene, false);
			++progress;
		}
		break;
	case 5: ++progress; break;
	default: break;
	}
}

void ReturnToBase::render(sf::RenderWindow& win, sf::Vector2f cam) {
	if (m_champion) { m_champion->render(win, cam); }
}

} // namespace fornani
