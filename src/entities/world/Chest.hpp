
#pragma once

#include <string>
#include "../../particle/Sparkler.hpp"
#include "../../utils/Collider.hpp"
#include "../Entity.hpp"
#include "../animation/Animation.hpp"
#include "../item/Item.hpp"

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

static inline anim::Parameters unopened{0, 1, 80, -1};
static inline anim::Parameters shine{1, 5, 24, 0};
static inline anim::Parameters opened{6, 1, 8, -1};

enum class ChestState {activated, open};
enum class ChestType {gun, orbs, item};

class Chest : public Entity {
  public:
	Chest(automa::ServiceProvider& svc, int id);
	void update(automa::ServiceProvider& svc, world::Map& map, gui::Console& console, player::Player& player);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> campos);
	void set_position(sf::Vector2<float> pos);
	void set_position_from_scaled(sf::Vector2<float> scaled_pos);
	void set_id(int new_id);
	void set_item(int to_id);
	void set_amount(int to_amount);
	void set_rarity(float to_rarity);
	void set_type(ChestType to_type);

  private:
	vfx::Sparkler sparkler{};
	anim::Animation animation{};
	shape::Collider collider{};

	util::BitFlags<ChestState> state{};
	ChestType type{};

	int id{};
	int item_id{};

	struct {
		int amount{};
		float rarity{};
	} loot{};
};

} // namespace entity
