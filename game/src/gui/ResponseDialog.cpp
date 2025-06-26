
#include "fornani/gui/ResponseDialog.hpp"
#include "fornani/audio/Soundboard.hpp"
#include "fornani/graphics/Colors.hpp"
#include "fornani/setup/ControllerMap.hpp"
#include "fornani/setup/TextManager.hpp"

namespace fornani::gui {

ResponseDialog::ResponseDialog(data::TextManager& text, dj::Json& source, std::string_view key, int index, sf::Vector2f start_position) : m_text_size{16}, m_selection{1}, m_index{index} {
	auto& set = source[key]["responses"][index];
	for (auto& msg : set.as_array()) {
		m_responses.push_back(sf::Text(text.fonts.basic));
		m_responses.back().setString(msg.as_string().data());
		stylize(m_responses.back());
	}
	m_selection = util::Circuit{static_cast<int>(m_responses.size())};
	m_indicator.shape.setSize({4.f, 4.f});
	m_indicator.shape.setFillColor(colors::bright_orange);
	m_indicator.shape.setOrigin({2.f, 2.f});
	m_indicator.shape.setPosition(start_position);
	m_indicator.position = start_position;
	m_indicator.physics.position = start_position;
	m_position = start_position;
}

void ResponseDialog::stylize(sf::Text& message) const {
	message.setCharacterSize(m_text_size);
	message.setFillColor(colors::ui_white);
	message.setLineSpacing(1.5f);
}

bool ResponseDialog::handle_inputs(config::ControllerMap& controller, audio::Soundboard& soundboard) {
	auto const& up = controller.digital_action_status(config::DigitalAction::menu_up).triggered;
	auto const& down = controller.digital_action_status(config::DigitalAction::menu_down).triggered;
	auto const& select = controller.digital_action_status(config::DigitalAction::menu_select).triggered;

	if (select && m_ready) {
		soundboard.flags.console.set(audio::Console::next);
		return false;
	}
	if (up) {
		soundboard.flags.console.set(audio::Console::shift);
		m_selection.modulate(-1);
	}
	if (down) {
		soundboard.flags.console.set(audio::Console::shift);
		m_selection.modulate(1);
	}
	m_ready = true;
	return true;
}

void ResponseDialog::render(sf::RenderWindow& win) {
	auto ctr{0.f};
	for (auto& res : m_responses) {
		auto is_current = ctr == m_selection.as<float>();
		is_current ? res.setFillColor(colors::ui_white) : res.setFillColor(colors::blue);
		auto xoff = is_current ? 0.f : 4.f;
		res.setPosition(m_position + sf::Vector2f{xoff, 24.f * ctr});
		win.draw(res);
		if (is_current) {
			m_indicator.position = res.getPosition() + sf::Vector2f{-16.f, res.getLocalBounds().getCenter().y};
			m_indicator.shape.setPosition(m_indicator.physics.position);
			win.draw(m_indicator.shape);
		}
		++ctr;
	}
}

void ResponseDialog::update() { m_indicator.update(); }

void ResponseDialog::set_position(sf::Vector2f to_position) { m_position = to_position; }

void ResponseIndicator::update() {
	steering.target(physics, position, 0.01f);
	physics.simple_update();
}

} // namespace fornani::gui
