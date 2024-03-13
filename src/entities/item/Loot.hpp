
#pragma once

#include <string>
#include "Drop.hpp"

namespace automa {
struct ServiceProvider;
}

namespace item {

class Loot {

	using Vec = sf::Vector2<float>;
	using Vecu16 = sf::Vector2<uint32_t>;

  public:
	Loot() = default;
	Loot(automa::ServiceProvider& svc, sf::Vector2<int> drop_range, float probability, sf::Vector2<float> pos);

	void update();
	void render(sf::RenderWindow& win, sf::Vector2<float> campos);
	void set_position(sf::Vector2<float> pos);

	std::vector<Drop>& get_drops();

  private:
	sf::Vector2<float> position{};
	std::vector<Drop> drops{};
};

} // namespace item
