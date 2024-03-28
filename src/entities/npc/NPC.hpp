
#pragma once

#include <string>
#include "../../utils/Collider.hpp"
#include "../Entity.hpp"
#include "NPCAnimation.hpp"
#include <string_view>

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

enum class NPCState { engaged };

class NPC : public Entity {
  public:
	NPC(automa::ServiceProvider& svc, int id);
	void update(automa::ServiceProvider& svc, world::Map& map, gui::Console& console, player::Player& player);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> campos);
	void set_position(sf::Vector2<float> pos);
	void set_position_from_scaled(sf::Vector2<float> scaled_pos);
	void set_id(int new_id);

  private:
	NPCAnimation animation{};

	shape::Collider collider{};
	util::BitFlags<NPCState> state_flags{};

	std::string_view label{};
	std::string_view conversation{};
	int id{};
};

} // namespace entity
