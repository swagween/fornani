#pragma once

#include <SFML/Graphics.hpp>
#include <string_view>
#include "../utils/Cooldown.hpp"
#include "../utils/BitFlags.hpp"
#include "HelpText.hpp"

namespace automa {
struct ServiceProvider;
}

namespace text {

enum class TutorialFlags { jump, shoot, sprint };

class Tutorial {
  public:
	void update(automa::ServiceProvider& svc);
	void render(sf::RenderWindow& win);
	util::BitFlags<TutorialFlags> flags{};
	text::HelpText help_marker;
};

} // namespace flfx
