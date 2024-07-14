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

namespace entity {
enum class BedFlags { active };
class Bed {
  public:
	Bed(automa::ServiceProvider& svc, sf::Vector2<float> position);
	void update(automa::ServiceProvider& svc, world::Map& map, gui::Console& console, player::Player& player);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam);
	shape::Shape bounding_box{};
  private:
	vfx::Sparkler sparkler{};
	util::BitFlags<BedFlags> flags{};
	util::Cooldown fadeout{200};
};

} // namespace entity
