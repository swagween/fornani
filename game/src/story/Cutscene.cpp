
#include <fornani/automa/SceneContext.hpp>
#include <fornani/entities/player/Player.hpp>
#include <fornani/events/GameplayEvent.hpp>
#include <fornani/gui/console/Console.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/story/Cutscene.hpp>
#include <fornani/world/Map.hpp>

namespace fornani {

Cutscene::Cutscene(automa::ServiceProvider& svc, int id, std::string_view label) : id(id) {
	auto& in_data = svc.data.cutscene[label];
	metadata.no_player = static_cast<bool>(in_data["no_player"].as_bool());
	metadata.target_state_on_end = in_data["target_state_on_end"].as<int>();
	svc.events.set_cutscene_progression_event.attach_to(p_slot, &Cutscene::set_progress, this);
}

void Cutscene::update(automa::ServiceProvider& svc, SceneContext& context, world::Map& map, player::Player& player) {
	svc.state_flags.set(automa::StateFlags::hide_hud);
	svc.state_flags.set(automa::StateFlags::no_menu);
	svc.state_flags.set(automa::StateFlags::cutscene);
	svc.camera_controller.free();
	player.stall_idle_timer();
	cooldowns.beginning.update();
	cooldowns.pause.update();
	cooldowns.long_pause.update();
	cooldowns.end.update();
	player.controller.restrict_movement();
	if (context.console) { context.console.value()->set_no_exit(true); }
	if (cooldowns.end.is_almost_complete()) { flags.set(CutsceneFlags::complete); }
}

void Cutscene::end(automa::ServiceProvider& svc, player::Player& player) {
	player.controller.unrestrict();
	svc.state_flags.reset(automa::StateFlags::hide_hud);
	svc.state_flags.reset(automa::StateFlags::no_menu);
	svc.state_flags.reset(automa::StateFlags::cutscene);
	svc.camera_controller.constrain();
	svc.camera_controller.set_owner(graphics::CameraOwner::player);
	flags.set(CutsceneFlags::delete_me);
}

void Cutscene::set_progress(int const to) {
	if (progress >= to) { return; }
	progress = to;
	if (debug) { NANI_LOG_DEBUG(p_logger, "set cutscene progress to {}", to); }
}

} // namespace fornani
