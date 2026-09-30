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

enum class BattleArenaFlags : std::uint8_t { final_wave };

class BattleArena final : public Cutscene {
  public:
	explicit BattleArena(automa::ServiceProvider& svc, world::Map& map);
	void update(automa::ServiceProvider& svc, SceneContext& context, world::Map& map, player::Player& player) override;

  private:
	std::vector<EnemyWave> m_waves{};
	sf::Vector2f m_focus_point{};
	int m_destructible_id{};

	util::BitFlags<BattleArenaFlags> m_flags{};
};

} // namespace fornani
