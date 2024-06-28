
#pragma once

#include <SFML/Graphics.hpp>
#include <array>

namespace automa {
struct ServiceProvider;
}

namespace player {
class Player;
}

namespace flfx {

class Transition {

  public:
	Transition(automa::ServiceProvider& svc, int d);

	void update(player::Player& player);
	void render(sf::RenderWindow& win);

	int const duration{};
	int current_frame{};
	uint8_t alpha{255};
	bool done{false};
	bool fade_out{false};
	bool fade_in{false};
	int rate{4};

	sf::RectangleShape box{};
};

} // namespace flfx
