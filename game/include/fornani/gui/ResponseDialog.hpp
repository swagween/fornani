
#pragma once

#include "fornani/components/PhysicsComponent.hpp"
#include "fornani/components/SteeringBehavior.hpp"
#include "fornani/utils/Circuit.hpp"

#include <SFML/Graphics.hpp>
#include <djson/json.hpp>

#include <string_view>

namespace fornani::audio {
class Soundboard;
}

namespace fornani::config {
class ControllerMap;
}

namespace fornani::data {
class TextManager;
}

namespace fornani::gui {

struct ResponseIndicator {
	sf::Vector2f position{};
	components::PhysicsComponent physics{};
	components::SteeringBehavior steering{};
	sf::RectangleShape shape{};
	void update();
};

class ResponseDialog {
  public:
	ResponseDialog(data::TextManager& text, dj::Json& source, std::string_view key, int index = 0, sf::Vector2f start_position = {});
	/// @return true if dialog is still processing inputs, false when exit is requested
	bool handle_inputs(config::ControllerMap& controller, audio::Soundboard& soundboard);
	void render(sf::RenderWindow& win);
	void update();
	void set_position(sf::Vector2f to_position);
	[[nodiscard]] auto get_selection() const -> int { return m_selection.get(); }
	[[nodiscard]] auto get_index() const -> int { return m_index; }

  private:
	void stylize(sf::Text& message) const;
	std::vector<sf::Text> m_responses{};
	sf::Vector2f m_position{};
	ResponseIndicator m_indicator{};
	int m_text_size;
	int m_index{};
	bool m_ready{};
	util::Circuit m_selection;
};

} // namespace fornani::gui
