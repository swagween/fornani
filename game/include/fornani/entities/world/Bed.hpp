#pragma once

#include "fornani/entities/Entity.hpp"
#include "fornani/io/Logger.hpp"
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

namespace fornani::graphics {
class Transition;
}

namespace fornani::entity {
enum class BedFlags : std::uint8_t { active, engaged, slept_in };
class Bed {
  public:
	Bed(automa::ServiceProvider& svc, sf::Vector2f position, int style = 0, bool flipped = false);
	void update(automa::ServiceProvider& svc, world::Map& map, std::optional<std::unique_ptr<gui::Console>>& console, player::Player& player, graphics::Transition& transition);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam);
	shape::Shape bounding_box{};

  private:
	vfx::Sparkler sparkler;
	util::BitFlags<BedFlags> flags{};
	util::Cooldown fadeout{200};
	sf::Sprite m_sprite;

	io::Logger m_logger{"entity"};
};

} // namespace fornani::entity
