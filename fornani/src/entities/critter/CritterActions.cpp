
#include "../../setup/ServiceLocator.hpp"
#include "Critter.hpp"

namespace critter {

void Critter::random_walk(sf::Vector2<int> range) {
	// potentially initiate walk every [energy] frames
	if (svc::clockLocator.get().every_x_frames(stats.energy)) {
		// give a 30% chance to do a random walk
		if (svc::randomLocator.get().percent_chance(30) && abs(colliders.at(0).physics.velocity.x) < 1.0f) {
			auto distance = svc::randomLocator.get().random_range(range.x, range.y);
			current_target = sf::Vector2<float>(colliders.at(0).physics.position.x + (svc::randomLocator.get().signed_coin_flip() * distance), colliders.at(0).physics.position.y);
			flags.set(Flags::seeking);
			flags.set(Flags::running);
		}
	}
}

void Critter::random_idle_action() {

	if (svc::clockLocator.get().every_x_frames(10)) {
		if (svc::randomLocator.get().percent_chance(2)) { idle_action_queue.push(1); }
	}
}

} // namespace critter