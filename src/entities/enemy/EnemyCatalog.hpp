
#pragma once
#include "../enemy/catalog/Frdog.hpp"

namespace enemy {

class EnemyCatalog {
  public:
	EnemyCatalog() = default;
	EnemyCatalog(automa::ServiceProvider& svc);
	void update();
	void push_enemy(automa::ServiceProvider& svc, int id);

	std::vector<std::unique_ptr<enemy::Enemy>> enemies{};
};

} // namespace enemy