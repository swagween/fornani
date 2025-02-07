#pragma once

#include <string>
#include "fornani/utils/Shape.hpp"
#include "fornani/particle/Chain.hpp"
#include "fornani/utils/BitFlags.hpp"

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani::world {
class Map;
}

namespace fornani::player {
class Player;
}

namespace fornani::entity {
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

} // namespace fornani::entity
