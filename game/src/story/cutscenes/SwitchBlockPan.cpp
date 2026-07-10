
#include <fornani/automa/SceneContext.hpp>
#include <fornani/entities/player/Player.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/story/cutscene/SwitchBlockPan.hpp>
#include <fornani/world/Map.hpp>

namespace fornani {

fornani::SwitchBlockPan::SwitchBlockPan(automa::ServiceProvider& svc, int block_id) : Cutscene(svc, 2, "switch_block_pan"), m_block_id{block_id}, m_wait{256} {
	cooldowns.beginning.set_and_start(32);
	m_wait.start();
}

void SwitchBlockPan::update(automa::ServiceProvider& svc, SceneContext& context, world::Map& map, player::Player& player) {

	if (cooldowns.end.is_almost_complete()) {
		player.controller.unrestrict();
		svc.state_flags.reset(automa::StateFlags::no_menu);
		svc.state_flags.reset(automa::StateFlags::cutscene);
		svc.camera_controller.set_owner(graphics::CameraOwner::player);
		svc.camera_controller.constrain();
		flags.set(CutsceneFlags::delete_me);
		return;
	}

	svc.state_flags.set(automa::StateFlags::cutscene);
	svc.state_flags.set(automa::StateFlags::no_menu);
	cooldowns.beginning.update();
	cooldowns.pause.update();
	cooldowns.long_pause.update();
	cooldowns.end.update();
	m_wait.update();
	player.controller.restrict_movement();
	if (cooldowns.beginning.running()) { return; }

	auto campos = progress < 5 ? map.get_switch_block_position(m_block_id) : player.get_camera_focus_point();
	svc.camera_controller.set_owner(graphics::CameraOwner::system);
	svc.camera_controller.set_position(campos);

	switch (progress) {
	case 0:
		if (m_wait.is_almost_complete()) {
			cooldowns.pause.start();
			++progress;
		}
		break;
	case 1:
		if (cooldowns.pause.is_almost_complete()) { ++progress; }
		break;
	case 2:
		svc.data.activate_switch(m_block_id);
		cooldowns.long_pause.start(192);
		++progress;
		break;
	case 3:
		if (cooldowns.long_pause.is_complete()) {
			cooldowns.end.set_and_start(32);
			++progress;
		}
		break;
	default: break;
	}
}

} // namespace fornani
