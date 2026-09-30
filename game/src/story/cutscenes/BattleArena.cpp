
#include <fornani/automa/SceneContext.hpp>
#include <fornani/entities/player/Player.hpp>
#include <fornani/gui/console/Console.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/story/cutscene/BattleArena.hpp>
#include <fornani/utils/Math.hpp>
#include <fornani/world/Map.hpp>

namespace fornani {

BattleArena::BattleArena(automa::ServiceProvider& svc, world::Map& map) : Cutscene(svc, 999, "battle_arena") {
	cooldowns.beginning.set_and_start(100);
	svc.music_player.stop();

	auto const& in_data = svc.data.arenas[std::to_string(map.room_id)];
	svc.music_player.load(svc.finder, in_data["music"].as_string());
	m_destructible_id = in_data["destructible_id"].as<int>();
	auto const& in = in_data["waves"];
	for (auto const& wave : in.as_array()) {
		auto next = EnemyWave{};
		for (auto [i, list] : std::views::enumerate(wave["ids"].as_array())) {
			auto spawn = WaveSpawn{};
			spawn.id = list.as<int>();
			if (wave["variants"]) { spawn.variant = wave["variants"][i].as<int>(); }
			next.ids.push_back(spawn);
		}
		m_waves.push_back(next);
	}

	m_focus_point = util::average(map.home_points);
}

void BattleArena::update(automa::ServiceProvider& svc, SceneContext& context, world::Map& map, player::Player& player) {
	if (complete()) {
		Cutscene::end(svc, player);
		svc.music_player.resume();
		svc.data.switch_destructible_state(m_destructible_id, true);
		return;
	}

	if (cooldowns.beginning.is_almost_complete()) {
		svc.music_player.play_looped();
		svc.data.switch_destructible_state(m_destructible_id, true);
	}

	cooldowns.beginning.update();
	cooldowns.pause.update();
	cooldowns.long_pause.update();
	cooldowns.end.update();

	if (player.is_dead() || (m_flags.test(BattleArenaFlags::final_wave) && map.enemies_cleared() && cooldowns.long_pause.is_complete())) { cooldowns.end.start(); }
	if (map.enemies_cleared() && !cooldowns.long_pause.running()) { cooldowns.long_pause.start(); }
	if (cooldowns.long_pause.is_almost_complete()) {
		for (auto const& e : m_waves.back().ids) { map.spawn_enemy(e.id, map.get_random_home_point() + random::random_vector_float(-4.f, 4.f), e.variant, true); }
		if (!m_waves.empty()) { m_waves.pop_back(); }
		if (m_waves.empty()) { m_flags.set(BattleArenaFlags::final_wave); }
	}
	if (cooldowns.end.is_almost_complete()) { flags.set(CutsceneFlags::complete); }

	auto const campos = (player.get_camera_focus_point() + m_focus_point) * 0.5f;

	svc.camera_controller.free();
	svc.camera_controller.set_owner(graphics::CameraOwner::system);
	svc.camera_controller.set_position(campos);
}

} // namespace fornani
