
#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include "BitFlags.hpp"

namespace util {

// called once and reset before the end of the tick.
enum class Trigger { hook_found, hook_released };
// toggled depending on state
enum class State { hook_probing, hook_anchored, hook_snaking };

class Logger {
  public:
	void run();
	void handle_flags();
	void print_boolean(std::string label, bool test);
	BitFlags<Trigger> triggers{};
	BitFlags<State> states{};

	sf::Vector2<float> hook_bob_position{};
	sf::Vector2<float> hook_anchor_position{};
	sf::Vector2<float> hook_physics_position{};
	sf::Vector2<float> hook_projectile_box_position{};
	sf::Vector2<float> hook_projectile_physics_position{};
};

} // namespace util