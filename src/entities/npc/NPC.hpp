
#pragma once

#include <string>
#include "../../utils/Collider.hpp"
#include "../Entity.hpp"
#include "NPCAnimation.hpp"
#include <deque>
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

namespace npc {

enum class NPCState { engaged };

class NPC : public entity::Entity {
  public:
	NPC(automa::ServiceProvider& svc, int id);
	void update(automa::ServiceProvider& svc, world::Map& map, gui::Console& console, player::Player& player);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> campos);
	void set_position(sf::Vector2<float> pos);
	void set_position_from_scaled(sf::Vector2<float> scaled_pos);
	void set_id(int new_id);
	void push_conversation(std::string_view convo);

	std::string_view label{};

  private:
	std::unique_ptr<NPCAnimation> animation_machine{};
	shape::Collider collider{};
	util::BitFlags<NPCState> state_flags{};

	std::deque<std::string_view> conversations{};
	int id{};

	struct {
		float const walk_threshold{0.5f};
	} physical{};
};

} // namespace entity
