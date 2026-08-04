
#include <fornani/entities/player/Player.hpp>
#include <fornani/graphics/rewards/AbilityRewardSequence.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/world/Map.hpp>

namespace fornani::graphics {

AbilityRewardSequence::AbilityRewardSequence(automa::ServiceProvider& svc, player::Player& player, SceneContext& context)
	: IRewardSequence{svc, player, context}, m_sparkler{svc, sf::Vector2f{96.f, 96.f}, colors::ui_white, "item"}, m_cinematic{1000}, m_flash{280} {
	player.set_flag(player::PlayerFlags::in_reward_sequence);
	flags.set(RewardSequenceFlags::console_after_exit);
}

void AbilityRewardSequence::update(automa::ServiceProvider& svc, player::Player& player, world::Map& map) {
	IRewardSequence::update(svc, player, map);
	if (p_end.is_complete() && flags.test(RewardSequenceFlags::finished)) { return; }

	auto center = player.get_window_position();

	svc.camera_controller.free();
	svc.camera_controller.set_owner(CameraOwner::system);
	svc.state_flags.set(automa::StateFlags::hide_hud);
	svc.state_flags.set(automa::StateFlags::no_menu);
	svc.state_flags.set(automa::StateFlags::cutscene);
	svc.camera_controller.set_position(player.get_collider().get_center());

	m_sparkler.set_position(center - sf::Vector2f{48.f, 48.f});

	auto color = colors::ui_white;
	if (m_flash.running()) {
		auto cvec = std::vector{colors::ui_white, colors::transparent};
		color = gradient_color(cvec, m_flash.get_inverse_normalized());
		player.flat_shade(color);
	} else {
		player.set_flag(player::PlayerFlags::flat_shaded, false);
	}

	m_cinematic.update();
	m_flash.update();

	if (m_emitter) { m_emitter.value()->update(svc, map); }
	if (m_effect) {
		m_effect->update();
		if (m_effect->done()) { m_effect.reset(); }
	}

	if (p_start.is_almost_complete()) { m_cinematic.start(); }
	if (m_cinematic.get_normalized() == 0.8f) {
		m_emitter.emplace(std::make_unique<vfx::Emitter>(svc, center - sf::Vector2f{2.f, 2.f}, sf::Vector2f{4.f, 4.f}, "radiance"));
		svc.soundboard.play_sound("reward_sparkle");
		svc.soundboard.play_sound("reward_get");
		m_flash.start();
	}
	if (m_cinematic.get_normalized() == 0.82f) { m_effect.emplace(svc, "giga_flare", center); }
	if (m_cinematic.is_almost_complete()) {
		p_end.start();
		flags.set(RewardSequenceFlags::finished);
	}
	if (p_end.is_almost_complete()) {
		player.request_animation(player::AnimState::land);
		svc.state_flags.reset(automa::StateFlags::hide_hud);
		svc.state_flags.reset(automa::StateFlags::no_menu);
		svc.state_flags.reset(automa::StateFlags::cutscene);
		player.set_flag(player::PlayerFlags::in_reward_sequence, false);
		svc.camera_controller.constrain();
		svc.camera_controller.set_owner(CameraOwner::player);
	}
	player.simple_update();
}

void AbilityRewardSequence::render(sf::RenderWindow& win) {
	IRewardSequence::render(win);
	if (m_emitter) { m_emitter.value()->render(win, {}); }
	if (m_effect) { m_effect->render(win, {}); }
}

} // namespace fornani::graphics
