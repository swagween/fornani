#pragma once

#include <string>
#include "../../utils/Shape.hpp"
#include "../../particle/Chain.hpp"
#include "../../utils/BitFlags.hpp"

namespace automa {
struct ServiceProvider;
}

namespace world {
class Map;
}

namespace player {
class Player;
}

namespace entity {
	enum class VineFlags{foreground};
class Vine {
  public:
	Vine(automa::ServiceProvider& svc, sf::Vector2<float> position, int length = 5, int size = 1, bool foreground = true);
	void update(automa::ServiceProvider& svc, world::Map& map, player::Player& player);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam);
	[[nodiscard]] auto foreground() const -> bool { return flags.test(VineFlags::foreground); }

  private:
	util::BitFlags<VineFlags> flags{};
	sf::Vector2<float> position{};
	sf::Vector2<float> spacing{0.f, 24.f};
	int length{};
	int size{};
	sf::RectangleShape drawbox{};
	vfx::Chain chain;
	sf::Sprite sprite{};
	std::vector<std::array<int, 2>> encodings{};
};

} // namespace entity
