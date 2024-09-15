
#include "LadyNimbusIntro.hpp"
#include "../../gui/Console.hpp"
#include "../../level/Map.hpp"
#include "../../service/ServiceProvider.hpp"
#include "../../entities/player/Player.hpp"
#include <iostream>

namespace fornani {

LadyNimbusIntro::LadyNimbusIntro(automa::ServiceProvider& svc) : Cutscene(svc, 6001, "lady_nimbus_intro") {
	cooldowns.beginning.start();
}

void LadyNimbusIntro::update(automa::ServiceProvider& svc, gui::Console& console, world::Map& map, player::Player& player) {
	if (complete()) {
		map.transition.start();
		svc.state_controller.switch_rooms(122, metadata.target_state_on_end, map.transition);
		return;
	}

	svc.state_flags.set(automa::StateFlags::hide_hud);
	cooldowns.beginning.update();
	cooldowns.pause.update();
	cooldowns.long_pause.update();
	cooldowns.end.update();

	if (metadata.no_player) {
		player.set_position({3200.f, -64.f});
		player.controller.prevent_movement();
		player.collider.physics.zero_y();
	}

	if (debug) {
		if (svc.ticker.every_x_ticks(100)) { std::cout << "progress: " << progress << " / " << total_conversations << "\n"; }
		if (cooldowns.pause.get_cooldown() == 15) { std::cout << "Pause: "; }
		if (cooldowns.pause.running()) { std::cout << "."; }
		if (cooldowns.pause.get_cooldown() == 1) { std::cout << ". done.\n"; }
	}

	auto total_suites{0};
	for (auto& npc : map.npcs) { total_suites += npc.num_suites(); }
	total_conversations = std::max(total_conversations, total_suites);
	if (cooldowns.end.get_cooldown() == 1) {
		flags.set(CutsceneFlags::complete);
		std::cout << "done!\n";
		return;
	}
	if (map.npcs.size() < 2) { return; }
	if (cooldowns.beginning.running()) { return; }

	// get npcs
	auto& nimbus = *std::ranges::find_if(map.npcs, [](auto& n) { return n.get_id() == 22; });
	auto& hologus = *std::ranges::find_if(map.npcs, [](auto& n) { return n.get_id() == 23; });

	//dialog
	switch (progress) {
	case 0:
		if (console.is_complete()) {
			nimbus.force_engage();
			++progress;
			return;
		}
		break;
	case 1:
		if (cooldowns.long_pause.get_cooldown() == 256) { svc.soundboard.flags.transmission.set(audio::Transmission::statics); }
		if (console.is_complete() && !cooldowns.long_pause.running()) {
			cooldowns.long_pause.start();
			nimbus.pop_conversation();
			return;
		}
		if (console.is_complete() && cooldowns.long_pause.get_cooldown() == 1) {
			hologus.force_engage();
			++progress;
			return;
		}
		break;
	case 2:
		if (console.is_complete() && !cooldowns.pause.running()) {
			cooldowns.pause.start();
			hologus.pop_conversation();
			return;
		}
		if (console.is_complete() && cooldowns.pause.get_cooldown() == 1) {
			nimbus.force_engage();
			++progress;
			return;
		}
		break;
	case 3:
		if (console.is_complete() && !cooldowns.pause.running()) {
			cooldowns.pause.start();
			nimbus.pop_conversation();
			return;
		}
		if (console.is_complete() && cooldowns.pause.get_cooldown() == 1) {
			hologus.force_engage();
			++progress;
			return;
		}
		break;
	case 4:
		if (console.is_complete() && !cooldowns.pause.running()) {
			cooldowns.pause.start();
			hologus.pop_conversation();
			return;
		}
		if (console.is_complete() && cooldowns.pause.get_cooldown() == 1) {
			nimbus.force_engage();
			++progress;
			return;
		}
		break;
	case 5:
		if (console.is_complete() && !cooldowns.pause.running()) {
			cooldowns.pause.start();
			nimbus.pop_conversation();
			return;
		}
		if (console.is_complete() && cooldowns.pause.get_cooldown() == 1) {
			hologus.force_engage();
			++progress;
			return;
		}
		break;
	case 6:
		if (console.is_complete() && !cooldowns.pause.running()) {
			cooldowns.pause.start();
			hologus.pop_conversation();
			return;
		}
		if (console.is_complete() && cooldowns.pause.get_cooldown() == 1) {
			nimbus.force_engage();
			++progress;
			return;
		}
		break;
	case 7:
		if (console.is_complete() && !cooldowns.pause.running()) {
			cooldowns.pause.start();
			nimbus.pop_conversation();
			return;
		}
		if (console.is_complete() && cooldowns.pause.get_cooldown() == 1) {
			hologus.force_engage();
			++progress;
			return;
		}
		break;
	case 8:
		if (console.is_complete() && !cooldowns.pause.running()) {
			cooldowns.pause.start();
			hologus.pop_conversation();
			return;
		}
		if (console.is_complete() && cooldowns.pause.get_cooldown() == 1) {
			nimbus.force_engage();
			++progress;
			return;
		}
		break;
	case 9: 
		if (console.is_complete() && !cooldowns.end.running()) {
			cooldowns.end.start();
			nimbus.pop_conversation();
			return;
		}
		if (console.is_complete() && cooldowns.end.get_cooldown() == 1) {
			return;
		}
		break;
	default: break;
	}
}

} // namespace fornani