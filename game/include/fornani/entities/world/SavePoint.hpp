
#pragma once

#include <string>
#include "fornani/setup/EnumLookups.hpp"
#include "fornani/utils/Shape.hpp"
#include "fornani/entities/animation/Animation.hpp"
#include "fornani/particle/Sparkler.hpp"

namespace automa {
struct ServiceProvider;
}

namespace player {
class Player;
}

namespace gui {
class Console;
}

namespace entity {

	inline anim::Parameters anim_params{0, 12, 24, -1};

class SavePoint {

  public:
	using Vec = sf::Vector2<float>;
	using Vecu16 = sf::Vector2<uint32_t>;

	SavePoint() = default;
	SavePoint(automa::ServiceProvider& svc);

	void update(automa::ServiceProvider& svc, player::Player& player, gui::Console& console);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, Vec campos);

	void save(automa::ServiceProvider& svc, player::Player& player); // talk to SaveDataManager to write current progress to save.json

	Vec dimensions{32, 32};
	Vec sprite_dimensions{64.f, 64.f};
	Vec position{};
	Vecu16 scaled_position{};
	shape::Shape bounding_box{};
	shape::Shape proximity_box{};
	sf::Sprite sprite;
	sf::RectangleShape drawbox{}; // for debug
	anim::Animation animation{};
	vfx::Sparkler sparkler{};
	bool activated{};
	bool can_activate{true};

	int id{};

  private:
	int intensity{};
};

} // namespace entity
