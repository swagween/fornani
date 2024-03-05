
#pragma once

#include <string>
#include "../../setup/EnumLookups.hpp"
#include "../../utils/Shape.hpp"
#include "../behavior/Animation.hpp"
#include "../../particle/Emitter.hpp"

namespace entity {

	inline anim::Parameters anim_params{0, 10, 30, -1};

class SavePoint {

  public:
	using Vec = sf::Vector2<float>;
	using Vecu16 = sf::Vector2<uint32_t>;

	SavePoint();

	void update();
	void render(sf::RenderWindow& win, Vec campos);

	void save(); // talk to SaveDataManager to write current progress to save.json

	Vec dimensions{32, 32};
	Vec sprite_dimensions{64.f, 64.f};
	Vec position{};
	Vecu16 scaled_position{};
	shape::Shape bounding_box{};
	shape::Shape proximity_box{};
	sf::Sprite sprite{};
	anim::Animation animation{};
	vfx::Emitter sparkles{};
	vfx::EmitterStats sparkle_stats{};
	vfx::ElementBehavior sparkle_behavior{};
	bool activated{};
	bool can_activate{true};

	int id{};

  private:
	int intensity{};
};

} // namespace entity
