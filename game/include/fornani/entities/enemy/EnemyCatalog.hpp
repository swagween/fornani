
#pragma once

#include <fornani/core/Fwd.hpp>
#include <fornani/entities/enemy/EnemyRegistry.hpp>

namespace fornani::gui {
class Console;
}

namespace fornani::enemy {

class EnemyCatalog {
  public:
	explicit EnemyCatalog(automa::ServiceProvider& svc);
	void update();
	void push_enemy(automa::ServiceProvider& svc, world::Map& map, SceneContext& context, int id, EnemyParameters params = {0, {-1, 0}, {}, false});
	template <typename T>
	std::vector<T*> get_enemies() {
		std::vector<T*> ret;
		for (auto const& entity : enemies) {
			if (auto* e = dynamic_cast<T*>(entity.get())) { ret.push_back(e); }
		}
		return ret;
	}

	std::vector<std::unique_ptr<Enemy>> enemies{};

  private:
	EntityHandle m_next_handle{10000};
};

} // namespace fornani::enemy
