#pragma once

#include <memory>
#include <optional>
#include <vector>
#include "SpawnablePlatform.hpp"
#include "TreasureContainer.hpp"
#include "fornani/particle/Chain.hpp"
#include "fornani/utils/BitFlags.hpp"
#include "fornani/utils/Shape.hpp"

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
enum class VineFlags : std::uint8_t { foreground, reverse };
class Vine {
  public:
	Vine(automa::ServiceProvider& svc, sf::Vector2f position, int length = 5, int size = 1, bool foreground = true, bool reversed = false);
	void update(automa::ServiceProvider& svc, world::Map& map, player::Player& player);
	void on_hit(automa::ServiceProvider& svc, world::Map& map, arms::Projectile& proj) const;
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam);
	void add_platform(automa::ServiceProvider& svc, int link_index);
	[[nodiscard]] auto foreground() const -> bool { return flags.test(VineFlags::foreground); }

  private:
	std::optional<std::vector<std::unique_ptr<TreasureContainer>>> treasure_balls{};
	std::optional<std::vector<std::unique_ptr<SpawnablePlatform>>> spawnable_platforms{};
	util::BitFlags<VineFlags> flags{};
	sf::Vector2f position{};
	sf::Vector2f spacing{0.f, 24.f};
	int length{};
	int size{};
	sf::RectangleShape drawbox{};
	vfx::Chain chain;
	sf::Sprite sprite;
	std::vector<std::array<int, 2>> encodings{};
	struct {
		float priceless{0.001f};
		float rare{0.01f};
		float uncommon{0.1f};
		float special{0.001f};
	} constants{};
};

} // namespace fornani::entity
