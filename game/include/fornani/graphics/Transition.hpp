
#pragma once

#include <SFML/Graphics.hpp>
#include <array>
#include "fornani/utils/Cooldown.hpp"

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani::player {
class Player;
}

namespace fornani::flfx {

class Transition {

  public:
	Transition(automa::ServiceProvider& svc, int d);

	void update(player::Player& player);
	void render(sf::RenderWindow& win);
	void start();
	void end();
	[[nodiscard]] auto not_started() const -> bool { return !fade_out && !done; }
	[[nodiscard]] auto is_done() const -> bool { return done; }

  private:
	int duration{};
	util::Cooldown cooldown{};
	uint8_t alpha{255};
	bool done{};
	bool fade_out{};
	bool fade_in{};
	int rate{4};

	sf::RectangleShape box{};
	sf::Color color{};
};

} // namespace flfx
