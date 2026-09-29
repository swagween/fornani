
#include <fornani/automa/SceneContext.hpp>
#include <fornani/entities/player/Player.hpp>
#include <fornani/gui/console/Console.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/story/cutscene/BattleArena.hpp>
#include <fornani/world/Map.hpp>

namespace fornani {

BattleArena::BattleArena(automa::ServiceProvider& svc, world::Map& map) : Cutscene(svc, 999, "battle_arena") {
	cooldowns.beginning.set_and_start(100);
	svc.music_player.stop();
	auto const& in = svc.data.arenas[std::to_string(map.room_id)]["waves"];
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
}

void BattleArena::update(automa::ServiceProvider& svc, SceneContext& context, world::Map& map, player::Player& player) {
	if (complete()) {
		Cutscene::end(svc, player);
		svc.music_player.resume();
		return;
	}

	cooldowns.beginning.update();
	cooldowns.pause.update();
	cooldowns.long_pause.update();
	cooldowns.end.update();

	if (map.enemies_cleared() && !cooldowns.long_pause.running()) { cooldowns.long_pause.start(); }
	if (cooldowns.long_pause.is_almost_complete()) {
		for (auto const& e : m_waves.back().ids) { map.spawn_enemy(e.id, map.get_random_home_point(), e.variant); }
		m_waves.pop_back();
	}
}

} // namespace fornani
