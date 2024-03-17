#include <iostream>
#include "EnemyCatalog.hpp"
#include "../enemy/catalog/Frdog.hpp"

namespace enemy {

EnemyCatalog::EnemyCatalog(automa::ServiceProvider& svc) : frdog(svc) {}

void EnemyCatalog::push_critters(automa::ServiceProvider& svc, std::vector<std::pair<Enemy, int>> order) {
	for (auto& crit : order) {
		for (auto i{crit.second}; i > 0; --i) {
			enemy_pool.push_back(std::make_unique<Frdog>(svc));
			std::cout << "pushed\n";
		}
	}
}
} // namespace enemy
