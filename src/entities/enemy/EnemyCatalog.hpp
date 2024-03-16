
#pragma once
#include "../enemy/catalog/Frdog.hpp"

namespace enemy {

class EnemyCatalog {
  public:
	EnemyCatalog() = default;
	EnemyCatalog(automa::ServiceProvider& svc);
	void push_critters(automa::ServiceProvider& svc, std::vector<std::pair<Enemy, int>> order);

	std::vector<std::unique_ptr<Enemy>> enemy_pool{};

	Frdog frdog;
};

} // namespace enemy