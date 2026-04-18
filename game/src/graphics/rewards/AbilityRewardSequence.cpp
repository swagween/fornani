
#include <fornani/entities/player/Player.hpp>
#include <fornani/graphics/rewards/AbilityRewardSequence.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/world/Map.hpp>

namespace fornani::graphics {

AbilityRewardSequence::AbilityRewardSequence(automa::ServiceProvider& svc, player::Player& player, SceneContext& context)
	: IRewardSequence{svc, player, context}, m_sparkler{svc, sf::Vector2f{96.f, 96.f}, colors::ui_white, "item"}, m_cinematic{1000} {
	player.set_flag(player::PlayerFlags::in_reward_sequence);
	svc.camera_controller.free();
	flags.set(RewardSequenceFlags::console_after_exit);
}

void AbilityRewardSequence::update(automa::ServiceProvider& svc, player::Player& player, world::Map& map) {
	IRewardSequence::update(svc, player, map);

	auto center = player.get_window_position();

	m_sparkler.set_position(center - sf::Vector2f{48.f, 48.f});

	m_cinematic.update();

	player.simple_update();

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
	}
	if (m_cinematic.get_normalized() == 0.82f) { m_effect.emplace(svc, "giga_flare", center); }
	if (m_cinematic.is_almost_complete()) { p_end.start(); }
	if (p_end.is_almost_complete()) {
		player.request_animation(player::AnimState::land);
		player.set_flag(player::PlayerFlags::in_reward_sequence, false);
		svc.camera_controller.constrain();
	}
}

void AbilityRewardSequence::render(sf::RenderWindow& win) {
	IRewardSequence::render(win);
	if (m_emitter) { m_emitter.value()->render(win, {}); }
	if (m_effect) { m_effect->render(win, {}); }
}

} // namespace fornani::graphics
