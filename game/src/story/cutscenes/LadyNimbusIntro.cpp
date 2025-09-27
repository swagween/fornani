
#include "fornani/story/cutscene/LadyNimbusIntro.hpp"
#include <fornani/gui/console/Console.hpp>
#include "fornani/entities/player/Player.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/world/Map.hpp"

namespace fornani {

LadyNimbusIntro::LadyNimbusIntro(automa::ServiceProvider& svc) : Cutscene(svc, 601, "lady_nimbus_intro") { cooldowns.beginning.start(); }

void LadyNimbusIntro::update(automa::ServiceProvider& svc, std::optional<std::unique_ptr<gui::Console>>& console, world::Map& map, player::Player& player) {

	if (complete() && map.transition.is(graphics::TransitionState::inactive)) {
		map.transition.start();
		return;
	} else if (complete() && map.transition.is(graphics::TransitionState::black)) {
		svc.state_controller.switch_rooms(199, metadata.target_state_on_end, map.transition);
		svc.state_flags.reset(automa::StateFlags::no_menu);
		svc.state_flags.reset(automa::StateFlags::cutscene);
		return;
	}

	svc.state_flags.set(automa::StateFlags::hide_hud);
	svc.state_flags.set(automa::StateFlags::no_menu);
	svc.state_flags.set(automa::StateFlags::cutscene);
	cooldowns.beginning.update();
	cooldowns.pause.update();
	cooldowns.long_pause.update();
	cooldowns.end.update();

	if (metadata.no_player) {
		player.set_position({3200.f, -64.f});
		player.controller.restrict_movement();
		player.collider.physics.zero_y();
	}

	// TODO: put camera controls here
	svc.camera_controller.constrain();

	if (console) { console.value()->set_no_exit(true); }

	auto total_suites{0};
	for (auto& npc : map.npcs) { total_suites += npc->num_suites(); }
	total_conversations = std::max(total_conversations, total_suites);
	if (cooldowns.end.get() == 1) {
		flags.set(CutsceneFlags::complete);
		return;
	}
	if (map.npcs.size() < 2) { return; }
	if (cooldowns.beginning.running()) { return; }

	// get npcs
	auto& nimbus = *std::ranges::find_if(map.npcs, [](auto& n) { return n->get_id() == 22; });
	auto& hologus = *std::ranges::find_if(map.npcs, [](auto& n) { return n->get_id() == 23; });

	// dialog
	switch (progress) {
	case 0:
		if (!console) {
			nimbus->force_engage();
			++progress;
			return;
		}
		break;
	case 1:
		if (cooldowns.long_pause.get() == 256) { svc.soundboard.flags.transmission.set(audio::Transmission::statics); }
		if (!console && !cooldowns.long_pause.running()) {
			cooldowns.long_pause.start();
			nimbus->pop_conversation();
			return;
		}
		if (!console && cooldowns.long_pause.get() == 1) {
			hologus->force_engage();
			++progress;
			return;
		}
		break;
	case 2:
		if (!console && !cooldowns.pause.running()) {
			cooldowns.pause.start();
			hologus->pop_conversation();
			return;
		}
		if (!console && cooldowns.pause.get() == 1) {
			nimbus->force_engage();
			++progress;
			return;
		}
		break;
	case 3:
		if (!console && !cooldowns.pause.running()) {
			cooldowns.pause.start();
			nimbus->pop_conversation();
			return;
		}
		if (!console && cooldowns.pause.get() == 1) {
			hologus->force_engage();
			++progress;
			return;
		}
		break;
	case 4:
		if (!console && !cooldowns.pause.running()) {
			cooldowns.pause.start();
			hologus->pop_conversation();
			return;
		}
		if (!console && cooldowns.pause.get() == 1) {
			nimbus->force_engage();
			++progress;
			return;
		}
		break;
	case 5:
		if (!console && !cooldowns.pause.running()) {
			cooldowns.pause.start();
			nimbus->pop_conversation();
			return;
		}
		if (!console && cooldowns.pause.get() == 1) {
			hologus->force_engage();
			++progress;
			return;
		}
		break;
	case 6:
		if (!console && !cooldowns.pause.running()) {
			cooldowns.pause.start();
			hologus->pop_conversation();
			return;
		}
		if (!console && cooldowns.pause.get() == 1) {
			nimbus->force_engage();
			++progress;
			return;
		}
		break;
	case 7:
		if (!console && !cooldowns.pause.running()) {
			cooldowns.pause.start();
			nimbus->pop_conversation();
			return;
		}
		if (!console && cooldowns.pause.get() == 1) {
			hologus->force_engage();
			++progress;
			return;
		}
		break;
	case 8:
		if (!console && !cooldowns.pause.running()) {
			cooldowns.pause.start();
			hologus->pop_conversation();
			return;
		}
		if (!console && cooldowns.pause.get() == 1) {
			nimbus->force_engage();
			++progress;
			return;
		}
		break;
	case 9:
		if (!console && !cooldowns.end.running()) {
			cooldowns.end.start();
			nimbus->pop_conversation();
			return;
		}
		if (!console && cooldowns.end.get() == 1) { return; }
		break;
	default: break;
	}
}

} // namespace fornani
