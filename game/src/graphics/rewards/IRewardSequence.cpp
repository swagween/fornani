
#include <fornani/graphics/rewards/IRewardSequence.hpp>
#include <fornani/world/Map.hpp>

namespace fornani::graphics {

IRewardSequence::IRewardSequence(automa::ServiceProvider& svc, player::Player& player, world::Map& map) : p_end{250}, p_linger{400}, p_start{200} {
	p_start.start();
	map.transition.start();
}

void IRewardSequence::update(automa::ServiceProvider& svc, player::Player& player, world::Map& map) {
	p_start.update();
	p_linger.update();
	p_end.update();
}

void IRewardSequence::render(sf::RenderWindow& win) {}

} // namespace fornani::graphics
