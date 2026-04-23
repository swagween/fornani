
#include <fornani/entities/player/Player.hpp>
#include <fornani/graphics/rewards/HealthRewardSequence.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/world/Map.hpp>

namespace fornani::graphics {

constexpr auto max_shards_v = 4;

HealthRewardSequence::HealthRewardSequence(automa::ServiceProvider& svc, player::Player& player, SceneContext& context, sf::Vector2f hud_point)
	: IRewardSequence{svc, player, context}, Animatable{svc, "health_increase", {54, 47}}, m_case{svc, "health_increase_case", {54, 47}}, m_ui_heart{svc, "heads_up_display_hearts", {11, 11}},
	  m_status{(player.get_item_count("cridium_shard") - 1) % max_shards_v}, m_effect{}, m_case_path{svc.finder, std::filesystem::path{"/data/gui/console_paths.json"}, "health_increase_case", 360, util::InterpolationType::ease},
	  m_shard_path{svc.finder, std::filesystem::path{"/data/gui/console_paths.json"}, "health_increase_shard", 360, util::InterpolationType::ease}, m_cinematic{800}, m_heart_path{400}, m_render_point{svc.window->f_center_screen()},
	  m_sparkler{svc, sf::Vector2f{96.f, 96.f}, colors::ui_white, "item"}, m_hud_point{hud_point} {
	m_case.center();
	m_ui_heart.center();
	center();
	m_case_path.set_section("in");
	m_shard_path.set_section("in");
	push_animation("slot", {1, 3, 32, 0});
	push_and_set_animation("standard", {0, 1, 32, -1});

	m_case.push_animation("spin", {1, 6, 16, -1});
	m_case.push_and_set_animation("standard", {0, 1, 32, -1});

	m_ui_heart.push_and_set_animation("standard", {0, 1, 32, 0});
	m_ui_heart.set_channel(1);

	// svc.music_player.quick_play(svc.finder, "cridium");
}

void HealthRewardSequence::update(automa::ServiceProvider& svc, player::Player& player, world::Map& map) {
	IRewardSequence::update(svc, player, map);
	m_case_path.update();
	m_shard_path.update();
	m_sparkler.set_position(m_render_point - sf::Vector2f{48.f, 48.f} + m_case_path.get_position());
	if (p_start.running()) { return; }

	tick();
	m_case.tick();

	m_cinematic.update();
	if (!p_linger.running()) { m_heart_path.update(); }

	if (p_end.is_almost_complete() && has_flag_set(HealthRewardSequenceFlags::cinematic)) { svc.soundboard.play_sound("gem_get"); }

	if (m_emitter) { m_emitter.value()->update(svc, map); }
	m_sparkler.update(svc);

	if (m_case.is_animation("spin")) {
		if (m_cinematic.get_normalized() == 0.975f) { svc.soundboard.play_sound("heart_spin"); }
		m_case.set_framerate(16 - static_cast<int>(15.f * m_cinematic.get_inverse_quadratic_normalized()));
		if (m_cinematic.is_almost_complete()) {
			set_flag(HealthRewardSequenceFlags::show_heart);
			m_effect.emplace(svc, "giga_flare", m_case.get_window_position());
			svc.soundboard.play_sound("reward_get");
			svc.soundboard.play_sound("reward_sparkle");
			p_linger.start();
			m_heart_path.start();
			m_emitter.emplace(std::make_unique<vfx::Emitter>(svc, m_render_point - sf::Vector2f{2.f, 2.f}, sf::Vector2f{4.f, 4.f}, "radiance"));
		}
	}
	if (m_heart_path.is_almost_complete()) {
		p_end.start();
		svc.soundboard.play_sound("heart_get");
		flags.set(RewardSequenceFlags::health_get);
	}
	if (m_effect) {
		m_effect->update();
		if (m_effect->done()) { m_effect.reset(); }
	}
	if (m_shard_path.completed_step(2) && !has_flag_set(HealthRewardSequenceFlags::slotted)) {
		p_linger.start();
		svc.soundboard.play_sound("pioneer_slot");
		svc.soundboard.play_sound("heart_collide");

		// health increase
		if (m_status == 3) {
			m_cinematic.start();
			set_flag(HealthRewardSequenceFlags::cinematic);
			svc.soundboard.play_sound("cridium_shard");
			m_case.set_animation("spin");
		}

		set_animation("slot");
		set_flag(HealthRewardSequenceFlags::slotted);
		m_effect.emplace(svc, "slow_flare", m_case.get_window_position(), sf::Vector2f{}, 1);
	}
	if (has_flag_set(HealthRewardSequenceFlags::slotted)) {
		if (is_complete()) { set_animation("standard"); }
	}
	if (p_linger.is_almost_complete() && !has_flag_set(HealthRewardSequenceFlags::cinematic)) {
		m_case_path.set_section("out");
		m_shard_path.set_section("out");
		p_end.start();
	}
}

void HealthRewardSequence::render(sf::RenderWindow& win) {
	IRewardSequence::render(win);
	if (p_start.running()) { return; }
	if (!has_flag_set(HealthRewardSequenceFlags::show_heart)) {
		m_case.set_position(m_render_point + m_case_path.get_position());
		set_position(m_render_point + m_shard_path.get_position());
		win.draw(m_case);
		if (!has_flag_set(HealthRewardSequenceFlags::cinematic)) {
			set_channel(m_status);
			win.draw(*this);
			for (auto i = 0; i < m_status; ++i) {
				set_channel(i);
				set_position(m_case.get_window_position());
				win.draw(*this);
			}
		}
		m_sparkler.render(win, {});
	} else {
		m_ui_heart.set_position(util::vector_lerp(m_render_point, m_hud_point, m_heart_path.get_inverse_quadratic_normalized()));
		auto hide_heart = false;
		if (m_effect) {
			if (m_effect->animation.get_frame_count() < 2) { hide_heart = true; }
		}
		if (!hide_heart) { win.draw(m_ui_heart); }
	}
	if (m_emitter) { m_emitter.value()->render(win, {}); }
	if (m_effect) { m_effect->render(win, {}); }
}

} // namespace fornani::graphics
