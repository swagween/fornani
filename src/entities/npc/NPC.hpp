
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

enum class NPCState { engaged, force_interact, introduced };
enum class NPCTrigger { distant_interact };

class NPC : public entity::Entity {
  public:
	NPC(automa::ServiceProvider& svc, int id);
	void update(automa::ServiceProvider& svc, world::Map& map, gui::Console& console, player::Player& player);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> campos);
	void set_position(sf::Vector2<float> pos);
	void set_position_from_scaled(sf::Vector2<float> scaled_pos);
	void set_id(int new_id);
	void push_conversation(std::string_view convo);
	void flush_conversations();

	std::string_view label{};

  protected:
	util::BitFlags<NPCState> state_flags{};
	util::BitFlags<NPCTrigger> triggers{};
	std::deque<std::string_view> conversations{};
	shape::Collider collider{};
  private:
	std::unique_ptr<NPCAnimation> animation_machine{};

	int id{};

	struct {
		float walk_threshold{0.5f};
	} physical{};
};

} // namespace entity
