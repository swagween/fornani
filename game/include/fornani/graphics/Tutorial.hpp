#pragma once

#include <SFML/Graphics.hpp>
#include <string_view>
#include "fornani/utils/Cooldown.hpp"
#include "fornani/utils/BitFlags.hpp"
#include "HelpText.hpp"

namespace automa {
struct ServiceProvider;
}

namespace text {

enum class TutorialHelpers { trigger, render, closed };
enum class TutorialFlags { jump, sprint, inventory, shoot, map };

class Tutorial {
  public:
	Tutorial(automa::ServiceProvider& svc);
	void update(automa::ServiceProvider& svc);
	void render(sf::RenderWindow& win);
	void turn_on();
	void turn_off();
	void close_for_good();
	void trigger();
	[[nodiscard]] auto closed() const -> bool { return helpers.test(TutorialHelpers::closed); }
	[[nodiscard]] auto on() const -> bool { return helpers.test(TutorialHelpers::render); }

	util::BitFlags<TutorialFlags> flags{};
	util::BitFlags<TutorialHelpers> helpers{};
	TutorialFlags current_state{};
	text::HelpText help_marker;
	util::Cooldown maximum_display_time{4800};
};

} // namespace flfx
