#pragma once

#include <fornani/entities/world/SpawnablePlatform.hpp>
#include <fornani/entities/world/TreasureContainer.hpp>
#include <fornani/entity/Entity.hpp>
#include <fornani/particle/Chain.hpp>
#include <fornani/utils/BitFlags.hpp>
#include <memory>
#include <optional>
#include <vector>

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani::world {
class Map;
}

namespace fornani::player {
class Player;
}

namespace fornani {

enum class VineFlags : std::uint8_t { foreground, reverse };

class Vine : public Entity {
  public:
	Vine(automa::ServiceProvider& svc, dj::Json const& in);
	Vine(automa::ServiceProvider& svc, int length = 8, int size = 2, bool foreground = true, bool reversed = false);

	// std::unique_ptr<Entity> clone() const override;
	void serialize(dj::Json& out) override;
	void unserialize(dj::Json const& in) override;
	void expose() override;
	void update([[maybe_unused]] automa::ServiceProvider& svc, [[maybe_unused]] world::Map& map, [[maybe_unused]] std::optional<std::unique_ptr<gui::Console>>& console, [[maybe_unused]] player::Player& player) override;
	void render(sf::RenderWindow& win, sf::Vector2f cam, float size) override;

	void on_hit(automa::ServiceProvider& svc, world::Map& map, arms::Projectile& proj) const;
	void add_platform(automa::ServiceProvider& svc, int link_index);

	[[nodiscard]] auto is_foreground() const -> bool { return m_flags.test(VineFlags::foreground); }

  private:
	void init();
	std::optional<std::vector<std::unique_ptr<entity::TreasureContainer>>> m_treasure_balls{};
	std::optional<std::vector<std::unique_ptr<entity::SpawnablePlatform>>> m_spawnable_platforms{};
	util::BitFlags<VineFlags> m_flags{};
	sf::Vector2f m_spacing;
	int m_length{};
	vfx::Chain m_chain;
	std::vector<std::array<int, 2>> encodings{};
	struct {
		float priceless{0.001f};
		float rare{0.01f};
		float uncommon{0.1f};
		float special{0.001f};
	} constants{};

	automa::ServiceProvider* m_services;
};

} // namespace fornani
