#include "fornani/entities/enemy/EnemyCatalog.hpp"
#include "fornani/entities/enemy/boss/Minigus.hpp"
#include "fornani/entities/enemy/catalog/Archer.hpp"
#include "fornani/entities/enemy/catalog/Beamstalk.hpp"
#include "fornani/entities/enemy/catalog/Caster.hpp"
#include "fornani/entities/enemy/catalog/Demon.hpp"
#include "fornani/entities/enemy/catalog/Eyebit.hpp"
#include "fornani/entities/enemy/catalog/Eyebot.hpp"
#include "fornani/entities/enemy/catalog/Frdog.hpp"
#include "fornani/entities/enemy/catalog/Hulmet.hpp"
#include "fornani/entities/enemy/catalog/Imp.hpp"
#include "fornani/entities/enemy/catalog/Meatsquash.hpp"
#include "fornani/entities/enemy/catalog/Tank.hpp"
#include "fornani/entities/enemy/catalog/Thug.hpp"

namespace fornani::enemy {

EnemyCatalog::EnemyCatalog(automa::ServiceProvider& svc) {}

void EnemyCatalog::update() {
	std::erase_if(enemies, [this](auto const& e) { return e->gone(); });
}

void EnemyCatalog::push_enemy(automa::ServiceProvider& svc, world::Map& map, [[maybe_unused]] std::optional<std::unique_ptr<gui::Console>>& console, int id, bool spawned, int variant, sf::Vector2<int> start_direction) {
	switch (id) {
	case 0: enemies.push_back(std::make_unique<Hulmet>(svc, map)); break;
	case 1: enemies.push_back(std::make_unique<Tank>(svc, map, variant)); break;
	case 3: enemies.push_back(std::make_unique<Thug>(svc, map)); break;
	case 4: enemies.push_back(std::make_unique<Eyebot>(svc)); break;
	case 5: enemies.push_back(std::make_unique<Eyebit>(svc, spawned)); break;
	case 6: enemies.push_back(std::make_unique<Minigus>(svc, map, console)); break;
	case 7: enemies.push_back(std::make_unique<Demon>(svc, map)); break;
	case 8: enemies.push_back(std::make_unique<Caster>(svc, map)); break;
	case 9: enemies.push_back(std::make_unique<Archer>(svc, map)); break;
	case 10: enemies.push_back(std::make_unique<Beamstalk>(svc, map, start_direction)); break;
	case 11: enemies.push_back(std::make_unique<Meatsquash>(svc, map)); break;
	case 12: enemies.push_back(std::make_unique<Imp>(svc, map)); break;
	case 13: enemies.push_back(std::make_unique<Hulmet>(svc, map)); break;
	default: enemies.push_back(std::make_unique<Frdog>(svc)); break;
	}
}

} // namespace fornani::enemy
