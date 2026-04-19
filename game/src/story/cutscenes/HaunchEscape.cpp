
#include <fornani/automa/SceneContext.hpp>
#include <fornani/entities/player/Player.hpp>
#include <fornani/gui/console/Console.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/story/cutscene/HaunchEscape.hpp>
#include <fornani/world/Map.hpp>

namespace fornani {

HaunchEscape::HaunchEscape(automa::ServiceProvider& svc) : Cutscene(svc, 900, "haunch_intro"), m_intro{200} {
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
		svc.music_player.load("scuffle");
		svc.music_player.play_looped();
		player.set_flag(player::PlayerFlags::cutscene, false);
		svc.camera_controller.constrain();
		m_flags.set(HaunchEscapeFlags::over);
		return;
	}

	svc.state_flags.set(automa::StateFlags::hide_hud);
	svc.state_flags.set(automa::StateFlags::no_menu);
	svc.state_flags.set(automa::StateFlags::cutscene);
	cooldowns.beginning.update();
	cooldowns.pause.update();
	cooldowns.long_pause.update();
	cooldowns.end.update();

	player.controller.restrict_movement();
	player.stall_idle_timer();
	player.set_flag(player::PlayerFlags::show_weapon, false);

	auto npcs = map.get_entities<NPC>();
	auto hit = std::ranges::find_if(npcs, [](auto& n) { return n->get_specifier() == 39; });
	auto& haunch = *hit;

	auto haunch_enemy = map.get_enemy(28);
	if (haunch_enemy != nullptr) { haunch->set_world_position(haunch_enemy->get_collider().get_center()); }

	if (m_intro.just_started()) {
		if (context.console) { context.console.reset(); }
		player.controller.prevent_movement();
		player.get_collider().physics.zero_x();
		player.set_flag(player::PlayerFlags::cutscene);
		player.set_idle();
		haunch->set_flag(NPCFlags::cutscene);
		haunch->flush_conversations();
		haunch->push_conversation(3);
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

	svc.camera_controller.set_owner(graphics::CameraOwner::system);
	svc.camera_controller.free();
	svc.camera_controller.set_position(haunch->Mobile::get_global_center());

	switch (progress) {
	case 0:
		haunch->force_engage();
		++progress;
		break;
	case 1: break;
	case 2: break;
	case 3: break;
	case 4: break;
	case 10:
		if (!context.console.has_value()) { ++progress; }
		break;
	case 11: break;
	default: break;
	}
}

void HaunchEscape::render(sf::RenderWindow& win, sf::Vector2f cam) {}

} // namespace fornani
