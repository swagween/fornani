
#include "fornani/gui/ResponseDialog.hpp"
#include "fornani/graphics/Colors.hpp"
#include "fornani/setup/TextManager.hpp"

namespace fornani::gui {

ResponseDialog::ResponseDialog(data::TextManager& text, dj::Json& source, std::string_view key, int index) : m_text_size{16} {
	auto& set = source[key]["responses"][index];
	auto this_set = std::vector<sf::Text>{};
	for (auto& msg : set.array_view()) {
		this_set.push_back(sf::Text(text.fonts.basic));
		this_set.back().setString(msg.as_string().data());
		stylize(this_set.back());
	}
}

void ResponseDialog::stylize(sf::Text& message) const {
	message.setCharacterSize(m_text_size);
	message.setFillColor(colors::ui_white);
	message.setLineSpacing(1.5f);
}

} // namespace fornani::gui
