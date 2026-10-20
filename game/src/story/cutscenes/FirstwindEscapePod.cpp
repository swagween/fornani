
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

FirstwindEscapePod::FirstwindEscapePod(automa::ServiceProvider& svc, world::Map& map) : Cutscene(svc, 118, "firstwind_escape_pod"), m_shaker{{2.f, 4, 800}} {
	svc.input_system.flush_inputs();
	svc.camera_controller.set_owner(graphics::CameraOwner::player);
	svc.camera_controller.constrain();
	cooldowns.beginning.start();
	m_pod_physics.set_friction_componentwise({0.99f, 0.995f});
	m_pod_physics.maximum_velocity.y = 30.f;
}

void FirstwindEscapePod::update(automa::ServiceProvider& svc, SceneContext& context, world::Map& map, player::Player& player) {
	Cutscene::update(svc, context, map, player);
	if (cooldowns.beginning.just_started()) { context.console = std::make_unique<gui::Console>(svc, svc.text.basic, "escape_pod", gui::OutputType::no_exit); }

	auto interactables = map.get_entities<Interactable>();
	auto bit = std::ranges::find_if(interactables, [](auto& n) { return n->get_tag() == "escape_pod"; });
	auto& pod = *bit;

	if (progress < 7) { m_pod_physics.position = pod->get_world_position(); }
	if (map.room_id == 199 && context.transition.is_black()) { m_pod_physics.position.x = 840.f; }
	m_pod_physics.update(svc);
	auto const poff = sf::Vector2f{24.f, 38.f};
	auto const player_socket = progress != 6 ? m_pod_physics.position + poff : m_pod_physics.position + poff;

	if (map.room_id == 199) {
		m_pod_physics.maximum_velocity.y = 10.f;
		if (svc.ticker.every_x_ticks(20)) { m_jitter = random::vector_float(-40.f, 40.f); }
		auto camera_focus = player.get_collider().get_center() + sf::Vector2f{0.f, 100.f} + m_jitter;

		svc.camera_controller.set_owner(graphics::CameraOwner::system);
		svc.camera_controller.set_position(camera_focus);
	}
	if (map.room_id == 601) { Cutscene::end(svc, player); }

	if (progress > 4 && progress < 20) {
		player.set_position(player_socket);
		player.set_sitting();
		player.set_direction({LR::left});
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
			svc.soundboard.play_sound("escape_pod_open");

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
			m_pod->push_and_set_animation("close", {2, 3, 30, -1, true});
			svc.soundboard.play_sound("escape_pod_close");
			m_pod->set_origin({1.f, 1.f});
			cooldowns.long_pause.start();
			++progress;
		}
		break;
	case 5:
		if (cooldowns.long_pause.is_almost_complete()) {
			cooldowns.long_pause.start();
			svc.soundboard.play_sound("escape_pod_steam");
			auto const pod_center = pod->get_world_position() + sf::Vector2f{32.f, 48.f};
			map.spawn_effect(svc, "escape_pod_steam", pod_center);
			++progress;
		}
		break;
	case 6:
		if (cooldowns.long_pause.halfway()) {
			if (cooldowns.long_pause.get_normalized() > 0.49f) { m_shaker.shake(); }
			m_shaker.sustain();
			svc.soundboard.repeat_sound("rumble");
			m_shaker.set_point(m_pod_physics.position);
			m_shaker.tick(m_pod_physics.position);
		}
		if (cooldowns.long_pause.is_almost_complete()) {
			++progress;
			svc.data.switch_destructible_state(11702);
			svc.soundboard.play_sound("escape_pod_dislodge");
			svc.soundboard.play_sound("escape_pod_chute");
			svc.soundboard.play_sound("escape_pod_hatch");
		}
		break;
	case 7: m_pod_physics.acceleration.y = 1.5f; break;
	case 20: break;
	case 30: Cutscene::end(svc, player); break;
	}
}

void FirstwindEscapePod::render(sf::RenderWindow& win, sf::Vector2f cam) {
	if (m_pod) {
		m_pod->set_channel(1);
		m_pod->set_position(m_pod_physics.position - cam);
		win.draw(*m_pod);
		m_pod->set_channel(0);
		m_pod->set_position(m_pod_physics.position - cam);
		win.draw(*m_pod);
	}
	// debug_window();
}

void FirstwindEscapePod::render_on_top(sf::RenderWindow& win, sf::Vector2f cam) {
	if (m_pod) {
		m_pod->set_channel(0);
		m_pod->set_position(m_pod_physics.position - cam);
		win.draw(*m_pod);
	}
}

} // namespace fornani
