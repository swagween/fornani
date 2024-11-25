#pragma once

#include "../particle/Gravitator.hpp"
#include "../service/ServiceProvider.hpp"
#include "../utils/Circuit.hpp"

#include <SFML/Graphics.hpp>

#include <functional>
#include <string>
#include <string_view>

namespace gui {
struct Option  {
	sf::Text text;
	bool enabled{};
	std::function<void()> on_select;
};

class OptionList {
  public:
	OptionList() = default;
	OptionList(automa::ServiceProvider&);

	void push_option(std::string_view text, std::function<void()> on_select, bool enabled = true);

	void update(automa::ServiceProvider&, bool enable_input = true, bool update_action_ctx_bar = true);

    sf::FloatRect getLocalBounds() const { return bounds; };

	void render(automa::ServiceProvider&, sf::RenderWindow&, sf::Vector2f offset = {0.f, 0.f});

  private:
	sf::Font option_font;

	std::vector<Option> options;
	util::Circuit selected_option {1};

	vfx::Gravitator left_dot{};
	vfx::Gravitator right_dot{};

	sf::FloatRect bounds{};

	sf::Color deselected_color{};
	sf::Color hovering_color{};

	constexpr static float vertical_spacing = 16.f;
};

} // namespace gui