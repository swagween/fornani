#include "TextWriter.hpp"

#include <SFML/Graphics.hpp>
#include "../service/ServiceProvider.hpp"
#include "../setup/ServiceLocator.hpp"

namespace text {

void TextWriter::start() {
	working_message = message;
	// calculate number of lines and call wrap() that many times.
	// can't call wrap() tick-wise because it's very slow
	auto num_glyphs = message.getString().getSize();
	auto length = message.getCharacterSize() + message.getLineSpacing();
	auto gpl = bounds.x / length;
	auto num_lines = num_glyphs / gpl;
	for (int i = 0; i < num_lines; ++i) { wrap(); }

	activate();
}

void TextWriter::update() {

	if (!writing()) { return; }
	if (tick_count % writing_speed == 0) {
		char const next_char = (char)message.getString().getData()[glyph_count];
		working_str += next_char;
		working_message.setString(working_str);
		++glyph_count;
	}
	if (glyph_count >= message.getString().getSize()) {
		reset();
		deactivate();
	}
	++tick_count;
}

void TextWriter::set_position(sf::Vector2<float> pos) { position = pos; }

void TextWriter::set_bounds(sf::Vector2<float> new_bounds) { bounds = new_bounds; }

void TextWriter::wrap() {

	float horizontal_extent = position.x + message.getLocalBounds().width;
	if (horizontal_extent > bounds.x) {

		// get index of last in-bounds space
		int last_space_index{};

		svc::stopwatchLocator.get().start();
		for (auto i{0}; i < message.getString().getSize(); ++i) {
			char const current_char = (char)message.getString().getData()[i];
			if (std::isspace(current_char)) {
				if (message.findCharacterPos(i).x < bounds.x) {
					last_space_index = i;
				} else {
					// splice!
					std::string left = message.getString().substring(0, last_space_index);
					std::string right = message.getString().substring(last_space_index + 1, message.getString().getSize() - 1);
					left += '\n';

					message.setString(left + right);
					return;
				}
			}
		}
		svc::stopwatchLocator.get().stop();
	}
}

void TextWriter::load_message(automa::ServiceProvider& svc, dj::Json& source, std::string_view key) {
	font.loadFromFile(svc.text.font);
	suite.clear();

	for (auto& msg : source[key].array_view()) {
		suite.push_back(sf::Text());
		suite.back().setString(msg.as_string().data());
		stylize(suite.back());
	}

	message = suite.front();
	working_message = message;
}

void TextWriter::stylize(sf::Text& msg) {
	msg.setCharacterSize(text_size);
	msg.setFillColor(flcolor::ui_white);
	msg.setFont(font);
	msg.setPosition(position);
}

void TextWriter::write_instant_message(sf::RenderWindow& win) {
	win.draw(message);
}

void TextWriter::write_gradual_message(sf::RenderWindow& win) {
	if (!writing()) {
		win.draw(message);
		return;
	}
	win.draw(working_message);
}

void TextWriter::reset() {
	writing_speed = default_writing_speed;
	glyph_count = 0;
	tick_count = 0;
	working_message = {};
	working_str = {};
}

void TextWriter::skip_ahead() { writing_speed = fast_writing_speed; }

void TextWriter::activate() { flags.set(MessageState::writing); }

void TextWriter::deactivate() { flags.reset(MessageState::writing); }

void TextWriter::request_next() {
	if (writing()) { return; }
	if (suite.empty()) {
		reset();
	} else {
		suite.pop_front();
		if (suite.empty()) {
			reset();
			deactivate();
			return;
		}
		message = suite.front();
		reset();
		activate();
		start();
	}
}

bool text::TextWriter::writing() const { return flags.test(MessageState::writing); }

bool text::TextWriter::complete() const { return !flags.test(MessageState::writing) && suite.empty(); }

} // namespace text
