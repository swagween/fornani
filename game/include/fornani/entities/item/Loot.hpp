
#pragma once

#include <string>
#include "Drop.hpp"
#include "fornani/utils/BitFlags.hpp"

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani::player {
class Player;
}

namespace fornani::item {

enum class LootState { heart_dropped };

class Loot {

	using Vec = sf::Vector2<float>;
	using Vecu16 = sf::Vector2<std::uint32_t>;

  public:
	Loot() = default;
	Loot(automa::ServiceProvider& svc, sf::Vector2<int> drop_range, float probability, sf::Vector2<float> pos, int delay_time = 0, bool special = false, int special_id = 0);

	void update(automa::ServiceProvider& svc, world::Map& map, player::Player& player);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> campos);
	void set_position(sf::Vector2<float> pos);

  private:
	sf::Vector2<float> position{};
	std::vector<std::unique_ptr<Drop>> drops{};
	util::BitFlags<LootState> flags{};
};

} // namespace fornani::item
