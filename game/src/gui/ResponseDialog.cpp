
#include "fornani/gui/ResponseDialog.hpp"
#include "fornani/graphics/Colors.hpp"
#include "fornani/setup/ControllerMap.hpp"
#include "fornani/setup/TextManager.hpp"

namespace fornani::gui {

ResponseDialog::ResponseDialog(data::TextManager& text, dj::Json& source, std::string_view key, int index) : m_text_size{16}, m_selection{1}, m_index{index} {
	auto& set = source[key]["responses"][index];
	for (auto& msg : set.array_view()) {
		m_responses.push_back(sf::Text(text.fonts.basic));
		m_responses.back().setString(msg.as_string().data());
		stylize(m_responses.back());
	}
	m_selection = util::Circuit{static_cast<int>(m_responses.size())};
}

void ResponseDialog::stylize(sf::Text& message) const {
	message.setCharacterSize(m_text_size);
	message.setFillColor(colors::ui_white);
	message.setLineSpacing(1.5f);
}

bool ResponseDialog::handle_inputs(config::ControllerMap& controller) {
	auto const& up = controller.digital_action_status(config::DigitalAction::menu_up).triggered;
	auto const& down = controller.digital_action_status(config::DigitalAction::menu_down).triggered;
	auto const& select = controller.digital_action_status(config::DigitalAction::menu_select).triggered;

	if (select && m_ready) { return false; }
	if (up) { m_selection.modulate(-1); }
	if (down) { m_selection.modulate(1); }
	m_ready = true;
	return true;
}

void ResponseDialog::render(sf::RenderWindow& win) {
	auto pos = sf::Vector2f{300.f, 300.f};
	auto res_off = sf::Vector2f{0.f, 64.f};
	auto ctr{0.f};
	for (auto& res : m_responses) {
		res.setPosition(pos + res_off * ctr);
		ctr == m_selection.as<float>() ? res.setFillColor(colors::ui_white) : res.setFillColor(colors::blue);
		win.draw(res);
		++ctr;
	}
}

} // namespace fornani::gui
