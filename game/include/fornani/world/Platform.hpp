#pragma once
#include <SFML/Graphics.hpp>
#include <string_view>
#include "fornani/entities/animation/Animation.hpp"
#include "fornani/io/Logger.hpp"
#include "fornani/utils/Collider.hpp"
#include "fornani/utils/Counter.hpp"

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

enum class PlatformAttributes : std::uint8_t { sticky, loop, repeating, player_activated, player_controlled, up_down, side_to_side, ease };
enum class PlatformState : std::uint8_t { moving };

class Platform : public shape::Collider {
  public:
	Platform(automa::ServiceProvider& svc, sf::Vector2f position, sf::Vector2f dimensions, float extent, std::string_view specifications, float start_point = 0.f, int style = 0);
	void update(automa::ServiceProvider& svc, world::Map& map, player::Player& player);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam);
	void on_hit(automa::ServiceProvider& svc, world::Map& map, arms::Projectile& proj);
	void switch_directions();
	Direction direction{};
	util::Counter counter{};

  private:
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
	float path_length{};
	float path_position{};
	sf::Sprite sprite;
	anim::Animation animation{};
	util::Cooldown switch_up;
	int style{};
	int state{};
	sf::Vector2<int> offset{};

	io::Logger m_logger{"world"};
};
} // namespace fornani::world
