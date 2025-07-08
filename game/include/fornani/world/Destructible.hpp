
#pragma once

#include <SFML/Graphics.hpp>
#include <djson\json.hpp>
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

enum class DestroyerState : std::uint8_t { detonated };

class Destructible : public IWorldPositionable {
  public:
	Destructible(automa::ServiceProvider& svc, dj::Json const& in, int style_id = 0);
	void update(automa::ServiceProvider& svc, Map& map, player::Player& player);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam);
	void on_hit(automa::ServiceProvider& svc, Map& map, arms::Projectile& proj) const;
	shape::Shape& get_bounding_box();
	[[nodiscard]] auto detonated() const -> bool { return flags.test(DestroyerState::detonated); }

  private:
	int quest_id{};
	shape::Collider collider{};
	util::BitFlags<DestroyerState> flags{};
	sf::Sprite sprite;
};

} // namespace fornani::world
