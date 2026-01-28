
#pragma once

#include <SFML/Graphics.hpp>
#include <fornani/entity/Turret.hpp>
#include <fornani/graphics/Animatable.hpp>
#include <fornani/io/Logger.hpp>
#include <fornani/physics/RegisteredCollider.hpp>
#include <fornani/utils/Counter.hpp>
#include <string_view>

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

enum class PlatformAttributes { sticky, loop, repeating, player_activated, player_controlled, up_down, side_to_side, ease };
enum class PlatformState { moving };

class Platform : public Animatable {
  public:
	Platform(automa::ServiceProvider& svc, world::Map& map, sf::Vector2f position, sf::Vector2f dimensions, float extent, std::string_view specifications, float start_point = 0.f, int style = 0);
	void update(automa::ServiceProvider& svc, world::Map& map, player::Player& player);
	void post_update(automa::ServiceProvider& svc, world::Map& map, player::Player& player);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam);
	void on_hit(automa::ServiceProvider& svc, world::Map& map, arms::Projectile& proj);
	void switch_directions();
	void set_handle(EntityHandle to) { m_handle = to; }
	shape::Collider& get_collider() { return *m_collider.get(); }

	Direction direction{};
	util::Counter counter{};

	[[nodiscard]] auto is_sticky() const -> bool { return flags.attributes.test(PlatformAttributes::sticky); }
	[[nodiscard]] auto get_velocity() -> sf::Vector2f { return get_collider().physics.position - m_old_position; }

  private:
	shape::RegisteredCollider m_collider;
	std::optional<Turret*> m_turret{};

	struct {
		float horizontal{};
		float vertical{};
	} range{};

	struct {
		float speed{};
	} metrics{};

	struct {
		util::BitFlags<PlatformAttributes> attributes{};
		util::BitFlags<PlatformState> state{};
	} flags{};

	Direction native_direction{};

	sf::ConvexShape track_shape{};
	std::vector<sf::Vector2f> track{};
	sf::Vector2f m_old_position{};
	float path_length{};
	float path_position{};
	util::Cooldown switch_up;
	int style{};
	int state{};
	sf::Vector2<int> offset{};

	EntityHandle m_handle{};

	io::Logger m_logger{"world"};
};
} // namespace fornani::world
