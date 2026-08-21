
#pragma once

#include <SFML/Graphics.hpp>
#include <fornani/components/SteeringComponent.hpp>
#include <fornani/core/Fwd.hpp>
#include <fornani/utils/Cooldown.hpp>
#include <fornani/utils/NineSlice.hpp>

namespace fornani::gui {

class Tooltip {
  public:
	Tooltip(automa::ServiceProvider& svc, std::string_view style, std::string_view tag, sf::Vector2f anchor);
	void update();
	void render(sf::RenderWindow& win, sf::Vector2f cam, sf::Vector2f scale);

  private:
	util::NineSlice m_nineslice;
	sf::Vector2f m_anchor{};
	sf::Text m_text;
	util::Cooldown m_start;
};

} // namespace fornani::gui
