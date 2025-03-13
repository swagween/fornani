
#pragma once

#include "fornani/utils/Circuit.hpp"

#include <SFML/Graphics.hpp>
#include <djson/json.hpp>

#include <string_view>

namespace fornani::config {
class ControllerMap;
}

namespace fornani::data {
class TextManager;
}

namespace fornani::gui {

class ResponseDialog {
  public:
	ResponseDialog(data::TextManager& text, dj::Json& source, std::string_view key, int index = 0);
	/// @return true if dialog is still processing inputs, false when exit is requested
	bool handle_inputs(config::ControllerMap& controller);
	void render(sf::RenderWindow& win);
	[[nodiscard]] auto get_selection() const -> int { return m_selection.get(); }

  private:
	void stylize(sf::Text& message) const;
	std::vector<sf::Text> m_responses{};
	int m_text_size;
	bool m_ready{};
	util::Circuit m_selection;
};

} // namespace fornani::gui
