#pragma once

#include <string>
#include "../../utils/Shape.hpp"
#include "../../particle/Chain.hpp"
#include "../../utils/BitFlags.hpp"
#include "TreasureContainer.hpp"
#include <optional>
#include <vector>
#include <memory>

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
enum class VineFlags { foreground, reverse };
class Vine {
  public:
	Vine(automa::ServiceProvider& svc, sf::Vector2<float> position, int length = 5, int size = 1, bool foreground = true, bool reversed = false);
	void update(automa::ServiceProvider& svc, world::Map& map, player::Player& player);
	void on_hit(automa::ServiceProvider& svc, world::Map& map, arms::Projectile& proj);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam);
	[[nodiscard]] auto foreground() const -> bool { return flags.test(VineFlags::foreground); }

  private:
	std::optional<std::vector<std::unique_ptr<TreasureContainer>>> treasure_balls{};
	util::BitFlags<VineFlags> flags{};
	sf::Vector2<float> position{};
	sf::Vector2<float> spacing{0.f, 24.f};
	int length{};
	int size{};
	sf::RectangleShape drawbox{};
	vfx::Chain chain;
	sf::Sprite sprite{};
	std::vector<std::array<int, 2>> encodings{};
	struct {
		float priceless{0.001f};
		float rare{0.01f};
		float uncommon{0.1f};
		float special{0.001f};
	} constants{};
};

} // namespace entity
