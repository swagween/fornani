
#pragma once

#include "fornani/entities/animation/Animation.hpp"
#include "fornani/entities/item/Item.hpp"
#include "fornani/graphics/Animatable.hpp"
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

namespace fornani::entity {

enum class ChestState { activated, open };
enum class ChestType { gun, orbs, item };

class Chest final : public Animatable {
  public:
	Chest(automa::ServiceProvider& svc, int id, ChestType type, int modifier);
	void update(automa::ServiceProvider& svc, world::Map& map, std::optional<std::unique_ptr<gui::Console>>& console, player::Player& player);
	void render(sf::RenderWindow& win, sf::Vector2f cam);
	void set_position(sf::Vector2<float> pos);
	void set_position_from_scaled(sf::Vector2<float> scaled_pos);
	shape::Shape& get_jumpbox() { return collider.jumpbox; }
	shape::Collider& get_collider() { return collider; }

  private:
	shape::Collider collider{};

	util::BitFlags<ChestState> state{};
	ChestType m_type{};

	int m_id{};
	int m_content_modifier{};
	std::string m_item_label{};

	struct {
		anim::Parameters unopened{0, 1, 80, -1};
		anim::Parameters shine{1, 5, 24, 0};
		anim::Parameters opened{6, 1, 8, -1};
	} m_animations{};
};

} // namespace fornani::entity
