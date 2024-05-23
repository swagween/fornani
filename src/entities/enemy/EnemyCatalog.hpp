
#pragma once
#include "../enemy/catalog/Frdog.hpp"

namespace enemy {

class EnemyCatalog {
  public:
	EnemyCatalog() = default;
	EnemyCatalog(automa::ServiceProvider& svc);
	void update();
	void push_enemy(automa::ServiceProvider& svc, world::Map& map, int id);

	std::vector<std::unique_ptr<enemy::Enemy>> enemies{};
};

} // namespace enemy