
#pragma once

#include <SFML/Graphics.hpp>
#include <djson\json.hpp>
#include <fornani/io/Logger.hpp>
#include <fornani/utils/IWorldPositionable.hpp>
#include "fornani/utils/BitFlags.hpp"
#include "fornani/utils/Collider.hpp"

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani::player {
class Player;
}
namespace fornani::arms {
class Projectile;
}

namespace fornani::world {

class Map;

enum class DestructibleState { solid, destroyed };
enum class DestructibleAttributes { inverse, enemy_clear };

class Destructible : public IWorldPositionable {
  public:
	Destructible(automa::ServiceProvider& svc, dj::Json const& in, int style_id = 0);
	void update(automa::ServiceProvider& svc, Map& map, player::Player& player);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam);
	void on_hit(automa::ServiceProvider& svc, Map& map, arms::Projectile& proj);
	shape::Shape& get_bounding_box();

	[[nodiscard]] auto get_id() const -> int { return quest_id; }
	[[nodiscard]] auto is_inverse() const -> bool { return m_attributes.test(DestructibleAttributes::inverse); }
	[[nodiscard]] auto is_enemy_clear() const -> bool { return m_attributes.test(DestructibleAttributes::enemy_clear); }
	[[nodiscard]] auto is_solid() const -> bool { return static_cast<DestructibleState>(m_state) == DestructibleState::solid; }
	[[nodiscard]] auto is_destroyed() const -> bool { return static_cast<DestructibleState>(m_state) == DestructibleState::destroyed; }
	[[nodiscard]] auto ignore_updates() const -> bool;

  private:
	int quest_id{};
	shape::Collider collider{};
	int m_state{};
	util::BitFlags<DestructibleAttributes> m_attributes{};
	sf::Sprite sprite;

	io::Logger m_logger{"Destructible"};
};

} // namespace fornani::world
