
#include <fornani/automa/SceneContext.hpp>
#include <fornani/entities/player/Player.hpp>
#include <fornani/entity/AmbientProp.hpp>
#include <fornani/entity/Interactable.hpp>
#include <fornani/entity/NPC.hpp>
#include <fornani/gui/console/Console.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/story/cutscene/FirstwindEscapePod.hpp>
#include <fornani/story/cutscene/Landing.hpp>
#include <fornani/world/Map.hpp>

namespace fornani {

FirstwindEscapePod::FirstwindEscapePod(automa::ServiceProvider& svc, world::Map& map) : Cutscene(svc, 118, "firstwind_escape_pod") {
	svc.input_system.flush_inputs();
	svc.camera_controller.set_owner(graphics::CameraOwner::player);
	svc.camera_controller.constrain();
	cooldowns.beginning.start();
	m_pod_physics.set_friction_componentwise({0.99f, 0.98f});
}

void FirstwindEscapePod::update(automa::ServiceProvider& svc, SceneContext& context, world::Map& map, player::Player& player) {
	Cutscene::update(svc, context, map, player);
	if (cooldowns.beginning.just_started()) { context.console = std::make_unique<gui::Console>(svc, svc.text.basic, "escape_pod", gui::OutputType::no_exit); }

	auto interactables = map.get_entities<Interactable>();
	auto bit = std::ranges::find_if(interactables, [](auto& n) { return n->get_tag() == "escape_pod"; });
	auto& pod = *bit;

	if (progress < 7) { m_pod_physics.position = pod->get_world_position(); }
	m_pod_physics.update(svc);
	auto const player_socket = m_pod_physics.position + sf::Vector2f{32.f, 44.f};

	if (progress > 4) {
		player.set_position(player_socket);
		player.set_sitting();
		player.set_direction({LR::right});
	}

	if (m_pod) { m_pod->tick(); }

	switch (progress) {
	case 0: break;
	case 1:
		cooldowns.pause.start();
		++progress;
		break;
	case 2:
		if (cooldowns.pause.is_almost_complete()) {
			// open door
			pod->set_animation("open");
			cooldowns.pause.start();
			++progress;
		}
		break;
	case 3:
		if (cooldowns.pause.is_almost_complete()) {
			player.set_jumping();
			cooldowns.pause.start();
			++progress;
		}
		break;
	case 4:
		if (cooldowns.pause.is_almost_complete()) {
			pod->set_hidden(true);
			m_pod.emplace(Animatable{svc, "interactable_escape_pod", {34, 50}});
			m_pod->push_and_set_animation("close", {2, 2, 60, 0, true});
			m_pod->set_origin({-2.f, -2.f});
			cooldowns.long_pause.start();
			++progress;
		}
		break;
	case 5:
		if (cooldowns.long_pause.is_almost_complete()) {
			svc.data.switch_destructible_state(11702);
			cooldowns.long_pause.start();
			++progress;
		}
		break;
	case 6:
		if (cooldowns.long_pause.is_almost_complete()) { ++progress; }
		break;
	case 7: m_pod_physics.acceleration.y = 1.f; break;
	case 20: break;
	case 30: Cutscene::end(svc, player); break;
	}
}

void FirstwindEscapePod::render(sf::RenderWindow& win, sf::Vector2f cam) {
	if (m_pod) {
		m_pod->set_position(m_pod_physics.position - cam);
		win.draw(*m_pod);
	}
	// debug_window();
}

} // namespace fornani
