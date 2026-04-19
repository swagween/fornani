
#pragma once

#include <fornani/core/Fwd.hpp>
#include <fornani/entities/enemy/Enemy.hpp>

namespace fornani::enemy {

struct Multispawn {
	sf::Vector2f spread{};
};

struct EnemyParameters {
	int variant{};
	sf::Vector2<int> dir{};
	Multispawn multispawn{};
	bool spawned{};
};

using EnemyFactory = std::function<std::unique_ptr<Enemy>(automa::ServiceProvider&, world::Map&, SceneContext&, EnemyParameters)>;

class EnemyRegistry {
  private:
	static auto& get_map() {
		static std::unordered_map<int, EnemyFactory> map;
		return map;
	}

  public:
	static void register_factory(int id, EnemyFactory factory) { get_map()[id] = std::move(factory); }

	static std::unique_ptr<Enemy> create(int id, automa::ServiceProvider& svc, world::Map& map, SceneContext& ctx, EnemyParameters params) {
		auto& map_ref = get_map();
		auto it = map_ref.find(id);
		if (it == map_ref.end()) return nullptr;

		return it->second(svc, map, ctx, params);
	}
};

} // namespace fornani::enemy
