
#include <fornani/gui/console/ResponseDialog.hpp>
#include "fornani/audio/Soundboard.hpp"
#include "fornani/graphics/Colors.hpp"
#include "fornani/setup/ControllerMap.hpp"
#include "fornani/setup/TextManager.hpp"

namespace fornani::gui {

ResponseDialog::ResponseDialog(data::TextManager& text, dj::Json& source, std::string_view key, int index, sf::Vector2f start_position) : m_text_size{16}, m_selection{1}, m_index{index} {
	auto& set = key == null_key ? source["responses"][index] : source[key]["responses"][index];
	for (auto& msg : set.as_array()) {
		responses.push_back(Message{sf::Text(text.fonts.basic)});
		responses.back().data.setString(msg["message"].as_string().data());
		stylize(responses.back().data);
		if (msg["codes"].is_array()) {
			responses.back().codes = std::vector<MessageCode>{};
			for (auto const& code : msg["codes"].as_array()) { responses.back().codes->push_back(MessageCode{code}); }
		}
	}
	m_selection = util::Circuit{static_cast<int>(responses.size())};
	m_indicator.shape.setSize({4.f, 4.f});
	m_indicator.shape.setFillColor(colors::bright_orange);
	m_indicator.shape.setOrigin({2.f, 2.f});
	m_indicator.shape.setPosition(start_position);
	m_indicator.position = start_position;
	m_indicator.physics.position = start_position;
	m_position = start_position;
}

auto ResponseDialog::get_codes(std::size_t index) const -> std::optional<std::vector<MessageCode>> {
	if (index >= responses.size()) { return std::nullopt; }
	if (!responses.at(index).codes) { return std::nullopt; }
	return responses.at(index).codes;
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
	for (auto& response : responses) {
		auto& res = response.data;
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
