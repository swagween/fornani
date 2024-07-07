#include "Tutorial.hpp"
#include "../service/ServiceProvider.hpp"

namespace text {


void Tutorial::update(automa::ServiceProvider& svc) {

	if (!flags.test(TutorialFlags::jump)) { help_marker.init(svc, "Press [", "main_action", "] to jump."); }

}


void Tutorial::render(sf::RenderWindow& win) {
	if (!flags.test(TutorialFlags::jump)) { help_marker.render(win); }
}

} // namespace text
