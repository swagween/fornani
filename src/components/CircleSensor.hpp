
#pragma once

#include <SFML/Graphics.hpp>
#include "../utils/Shape.hpp"
#include "../utils/BitFlags.hpp"

namespace components {
enum class SensorState { active };
class CircleSensor {
  public:
	CircleSensor();
	CircleSensor(float radius);
	void render(sf::RenderWindow& win, sf::Vector2<float> cam);
	void set_position(sf::Vector2<float> position);
	sf::CircleShape bounds{};
	sf::CircleShape drawable{};
	bool within_bounds(shape::Shape& rect) const;

	auto activate() -> void { state.set(SensorState::active); }
	auto deactivate() -> void { state.reset(SensorState::active); }
	[[nodiscard]] auto active() const -> bool { return state.test(SensorState::active); }

  private:
	util::BitFlags<SensorState> state{};
};

} // namespace components
