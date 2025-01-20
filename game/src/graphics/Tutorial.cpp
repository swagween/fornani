#include "fornani/graphics/Tutorial.hpp"
#include "fornani/service/ServiceProvider.hpp"

namespace text {

Tutorial::Tutorial(automa::ServiceProvider& svc) : help_marker(svc) {}

void Tutorial::update(automa::ServiceProvider& svc) {
	if (!svc.tutorial()) {
		helpers.set(TutorialHelpers::closed);
		return;
	}
	maximum_display_time.update();
	if (helpers.test(TutorialHelpers::closed)) { return; }
	if (!helpers.consume(TutorialHelpers::trigger)) { return; }
	if (!flags.test(TutorialFlags::jump)) {
		help_marker = HelpText(svc, "Press [", config::DigitalAction::platformer_jump, "] to jump.", 80, true, true);
		maximum_display_time.start();
		return;
	}
	if (!flags.test(TutorialFlags::sprint)) {
		help_marker = HelpText(svc, "Hold [", config::DigitalAction::platformer_sprint, "] to sprint.", 80, true, true);
		maximum_display_time.start();
		return;
	}
	if (!flags.test(TutorialFlags::inventory)) {
		help_marker = HelpText(svc, "Press [", config::DigitalAction::platformer_open_inventory, "] to open inventory.", 200, true, true);
		maximum_display_time.start();
		return;
	}
	if (!flags.test(TutorialFlags::shoot)) {
		help_marker = HelpText(svc, "Press [", config::DigitalAction::platformer_shoot, "] to shoot.", 200, true, true);
		maximum_display_time.start();
		return;
	}
	if (!flags.test(TutorialFlags::map)) {
		help_marker = HelpText(svc, "Press [", config::DigitalAction::platformer_open_map, "] to open map.", 200, true, true);
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
