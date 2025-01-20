#pragma once
#include <SFML/Graphics.hpp>
#include <string_view>
#include "fornani/setup/EnumLookups.hpp"
#include "fornani/utils/Collider.hpp"
#include "fornani/utils/Counter.hpp"
#include "fornani/entities/animation/Animation.hpp"

namespace automa {
struct ServiceProvider;
}

namespace player {
class Player;
}

namespace arms {
class Projectile;
}

namespace world {

enum class PlatformAttributes { sticky, loop, repeating, player_activated, player_controlled, up_down, side_to_side };
enum class PlatformState { moving };

class Platform : public shape::Collider {
  public:
	Platform(automa::ServiceProvider& svc, sf::Vector2<float> position, sf::Vector2<float> dimensions, float extent, std::string_view specifications, float start_point = 0.f, int style = 0);
	void update(automa::ServiceProvider& svc, world::Map& map, player::Player& player);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam);
	void on_hit(automa::ServiceProvider& svc, world::Map& map, arms::Projectile& proj);
	void switch_directions();
	dir::Direction direction{};
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

	dir::Direction native_direction{};

	sf::ConvexShape track_shape{};
	std::vector<sf::Vector2<float>> track{};
	float path_length{};
	float path_position{};
	sf::Sprite sprite;
	anim::Animation animation{};
	util::Cooldown switch_up{3};
	int style{};
	int state{};
	sf::Vector2<int> offset{};
};
} // namespace world