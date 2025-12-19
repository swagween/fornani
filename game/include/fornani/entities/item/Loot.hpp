
#pragma once

#include <fornani/entities/item/Drop.hpp>
#include <fornani/utils/BitFlags.hpp>
#include <string>

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani::player {
class Player;
}

namespace fornani::world {
class Map;
}

namespace fornani::item {

enum class LootState { heart_dropped };

class Loot {
  public:
	Loot() = default;
	Loot(automa::ServiceProvider& svc, world::Map& map, player::Player& player, sf::Vector2<int> drop_range, float probability, sf::Vector2f pos, int delay_time = 0, bool special = false, int special_id = 0);

	void update(automa::ServiceProvider& svc, world::Map& map, player::Player& player);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f campos);
	void set_position(sf::Vector2f pos);

	[[nodiscard]] auto get_size() const -> std::size_t { return drops.size(); }

  private:
	sf::Vector2f position{};
	std::vector<std::unique_ptr<Drop>> drops{};
	util::BitFlags<LootState> flags{};
};

} // namespace fornani::item
