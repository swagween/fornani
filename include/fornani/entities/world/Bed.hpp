#pragma once

#include <string>
#include "../../particle/Sparkler.hpp"
#include "../../utils/Collider.hpp"
#include "../Entity.hpp"

namespace automa {
struct ServiceProvider;
}

namespace world {
class Map;
}

namespace gui {
class Console;
}

namespace player {
class Player;
}

namespace flfx {
class Transition;
}

namespace entity {
enum class BedFlags { active, engaged };
class Bed {
  public:
	Bed(automa::ServiceProvider& svc, sf::Vector2<float> position, int room);
	void update(automa::ServiceProvider& svc, world::Map& map, gui::Console& console, player::Player& player, flfx::Transition& transition);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam);
	shape::Shape bounding_box{};
  private:
	vfx::Sparkler sparkler{};
	util::BitFlags<BedFlags> flags{};
	util::Cooldown fadeout{200};
	int room{};
};

} // namespace entity
