#include "fornani/graphics/Tutorial.hpp"
#include "fornani/service/ServiceProvider.hpp"

namespace fornani::text {

Tutorial::Tutorial(automa::ServiceProvider& svc) : help_marker(svc) {}

void Tutorial::update(automa::ServiceProvider& svc) {
	// TODO: completely overhaul tutorial
}

void Tutorial::render(sf::RenderWindow& win) {
	if (maximum_display_time.is_complete()) { return; }
	if (helpers.test(TutorialHelpers::closed)) { return; }
	if (helpers.test(TutorialHelpers::render)) { help_marker.render(win); }
}

void Tutorial::turn_on() {
	helpers.set(TutorialHelpers::render);
	helpers.reset(TutorialHelpers::closed);
	help_marker.reset();
}

void Tutorial::turn_off() { helpers.reset(TutorialHelpers::render); }

void Tutorial::close_for_good() { helpers.set(TutorialHelpers::closed); }

void Tutorial::trigger() { helpers.set(TutorialHelpers::trigger); }

} // namespace text
