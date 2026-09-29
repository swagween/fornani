#pragma once

#include <fornani/story/Cutscene.hpp>

namespace fornani {

constexpr auto arena_id_v = 999;

struct WaveSpawn {
	int id{};
	int variant{};
};

struct EnemyWave {
	std::vector<WaveSpawn> ids{};
};

class BattleArena final : public Cutscene {
  public:
	explicit BattleArena(automa::ServiceProvider& svc, world::Map& map);
	void update(automa::ServiceProvider& svc, SceneContext& context, world::Map& map, player::Player& player) override;

  private:
	std::vector<EnemyWave> m_waves{};
};

} // namespace fornani
