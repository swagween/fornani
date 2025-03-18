#pragma once

#include "fornani/entities/Entity.hpp"
#include "fornani/particle/Sparkler.hpp"
#include "fornani/utils/Collider.hpp"

#include <optional>

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani::world {
class Map;
}

namespace fornani::gui {
class Console;
}

namespace fornani::player {
class Player;
}

namespace fornani::flfx {
class Transition;
}

namespace fornani::entity {
enum class BedFlags : std::uint8_t { active, engaged };
class Bed {
  public:
	Bed(automa::ServiceProvider& svc, sf::Vector2<float> position, int room);
	void update(automa::ServiceProvider& svc, world::Map& map, std::optional<std::unique_ptr<gui::Console>>& console, player::Player& player, flfx::Transition& transition);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam);
	shape::Shape bounding_box{};

  private:
	vfx::Sparkler sparkler{};
	util::BitFlags<BedFlags> flags{};
	util::Cooldown fadeout{200};
	int room{};
};

} // namespace fornani::entity
