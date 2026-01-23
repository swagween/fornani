
#pragma once

#include <fornani/entities/enemy/Enemy.hpp>

namespace fornani::gui {
class Console;
}

namespace fornani::enemy {

struct Multispawn {
	sf::Vector2f spread{};
};

class EnemyCatalog {
  public:
	explicit EnemyCatalog(automa::ServiceProvider& svc);
	void update();
	void push_enemy(automa::ServiceProvider& svc, world::Map& map, [[maybe_unused]] std::optional<std::unique_ptr<gui::Console>>& console, int id, bool spawned = false, int variant = 0, sf::Vector2<int> start_direction = {-1, 0},
					Multispawn multispawn = {});
	template <typename T>
	std::vector<T*> get_enemies() {
		std::vector<T*> ret;
		for (auto const& entity : enemies) {
			if (auto* e = dynamic_cast<T*>(entity.get())) { ret.push_back(e); }
		}
		return ret;
	}

	std::vector<std::unique_ptr<Enemy>> enemies{};
};

} // namespace fornani::enemy
