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
enum class GrassFlags { foreground };
enum class GrassType { normal, bush };
class Grass {
  public:
	Grass(automa::ServiceProvider& svc, sf::Vector2<float> position, int length = 8, int size = 1, bool foreground = true);
	void update(automa::ServiceProvider& svc, world::Map& map, player::Player& player);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam);
	[[nodiscard]] auto foreground() const -> bool { return flags.test(GrassFlags::foreground); }

  private:
	util::BitFlags<GrassFlags> flags{};
	sf::Vector2<float> position{};
	sf::Vector2<float> spacing{0.f, 24.f};
	int length{};
	int size{};
	GrassType type{};
	sf::RectangleShape drawbox{};
	vfx::Chain chain;
	sf::Sprite sprite;
	std::vector<std::array<int, 2>> encodings{};
	struct {
		float f_height{4.f};
		int height{4};
		int num_slices{8};
	} constants{};
};

} // namespace entity
