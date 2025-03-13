
#pragma once

#include <SFML/Graphics.hpp>
#include <djson/json.hpp>

#include <string_view>

namespace fornani::data {
class TextManager;
}

namespace fornani::gui {

class ResponseDialog {
  public:
	ResponseDialog(data::TextManager& text, dj::Json& source, std::string_view key, int index = 0);

  private:
	void stylize(sf::Text& message) const;
	std::vector<sf::Text> m_responses{};
	int m_text_size;
};

} // namespace fornani::gui
