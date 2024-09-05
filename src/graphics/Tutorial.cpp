#include "Tutorial.hpp"
#include "../service/ServiceProvider.hpp"

namespace text {

void Tutorial::update(automa::ServiceProvider& svc) {
	if (!svc.controller_map.hard_toggles.test(config::Toggles::tutorial)) {
		helpers.set(TutorialHelpers::closed);
		return;
	}
	maximum_display_time.update();
	if (helpers.test(TutorialHelpers::closed)) { return; }
	if (!helpers.consume(TutorialHelpers::trigger)) { return; }
	if (!flags.test(TutorialFlags::jump)) {
		help_marker.init(svc, "Press [", "main_action", "] to jump.", 80, true);
		maximum_display_time.start();
		return;
	}
	if (!flags.test(TutorialFlags::sprint)) {
		help_marker.init(svc, "Hold [", "sprint", "] to sprint.", 80, true);
		maximum_display_time.start();
		return;
	}
	if (!flags.test(TutorialFlags::inventory)) {
		help_marker.init(svc, "Press [", "menu_toggle", "] to open inventory.", 200, true);
		maximum_display_time.start();
		return;
	}
	if (!flags.test(TutorialFlags::shoot)) {
		help_marker.init(svc, "Press [", "secondary_action", "] to shoot.", 200, true);
		maximum_display_time.start();
		return;
	}
	if (!flags.test(TutorialFlags::map)) {
		help_marker.init(svc, "View map from inventory by pressing [", "menu_toggle", "].", 200, true);
		maximum_display_time.start();
		return;
	}
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
