
#pragma once

#include "fornani/particle/Sparkler.hpp"
#include "fornani/utils/Collider.hpp"
#include "fornani/entities/Entity.hpp"
#include "fornani/entities/animation/Animation.hpp"
#include "fornani/entities/item/Item.hpp"

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

namespace fornani::entity {

// TODO: Do these really need to be globally accessible?

static inline anim::Parameters unopened{0, 1, 80, -1};
static inline anim::Parameters shine{1, 5, 24, 0};
static inline anim::Parameters opened{6, 1, 8, -1};

enum class ChestState {activated, open};
enum class ChestType {gun, orbs, item};

class Chest final : public Entity {
  public:
	Chest(automa::ServiceProvider& svc, int id);
	void update(automa::ServiceProvider& svc, world::Map& map, gui::Console& console, player::Player& player);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> campos) override;
	void set_position(sf::Vector2<float> pos);
	void set_position_from_scaled(sf::Vector2<float> scaled_pos);
	void set_id(int new_id);
	void set_item(int to_id);
	void set_amount(int to_amount);
	void set_rarity(float to_rarity);
	void set_type(ChestType to_type);
	shape::Shape& get_jumpbox() { return collider.jumpbox; }
	shape::Collider& get_collider() { return collider; }

  private:
	vfx::Sparkler sparkler{};
	anim::Animation animation{};
	shape::Collider collider{};
	sf::Sprite sprite;

	util::BitFlags<ChestState> state{};
	ChestType type{};

	int id{};
	int item_id{};

	struct {
		int amount{};
		float rarity{};
	} loot{};
};

} // namespace fornani::entity
