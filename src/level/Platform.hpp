#pragma once
#include <SFML/Graphics.hpp>
#include <string_view>
#include "../setup/EnumLookups.hpp"
#include "../utils/Collider.hpp"
#include "../utils/Counter.hpp"
#include "../entities/animation/Animation.hpp"

namespace automa {
struct ServiceProvider;
}

namespace player {
class Player;
}

namespace world {

	enum class PlatformAttributes { sticky, loop, repeating, player_activated, player_controlled };

class Platform : public shape::Collider {
  public:
	Platform(automa::ServiceProvider& svc, sf::Vector2<float> position, sf::Vector2<float> dimensions, float extent, std::string_view specifications, float start_point = 0.f, int style = 0);
	void update(automa::ServiceProvider& svc, player::Player& player);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam);
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
	} flags{};

	sf::ConvexShape track_shape{};
	std::vector<sf::Vector2<float>> track{};
	float path_length{};
	float path_position{};
	sf::Sprite sprite{};
	anim::Animation animation{};
	int style{};
	int state{};
	sf::Vector2<int> offset{};

};
} // namespace world