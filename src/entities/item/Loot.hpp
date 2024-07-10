
#pragma once

#include <string>
#include "Drop.hpp"
#include "../../utils/BitFlags.hpp"

namespace automa {
struct ServiceProvider;
}

namespace player {
class Player;
}

namespace item {

enum class LootState { heart_dropped };

class Loot {

	using Vec = sf::Vector2<float>;
	using Vecu16 = sf::Vector2<uint32_t>;

  public:
	Loot() = default;
	Loot(automa::ServiceProvider& svc, sf::Vector2<int> drop_range, float probability, sf::Vector2<float> pos, int delay_time = 0);

	void update(automa::ServiceProvider& svc, world::Map& map, player::Player& player);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> campos);
	void set_position(sf::Vector2<float> pos);

  private:
	sf::Vector2<float> position{};
	std::vector<Drop> drops{};
	util::BitFlags<LootState> flags{};
};

} // namespace item
