#include "TextWriter.hpp"

#include <SFML/Graphics.hpp>
#include "../service/ServiceProvider.hpp"
#include "../setup/ServiceLocator.hpp"

namespace text {

void TextWriter::start() {
	message.setCharacterSize(text_size);
	working_message.setCharacterSize(text_size);

	// calculate number of lines and call wrap() that many times.
	// can't call wrap() tick-wise because it's very slow
	auto num_glyphs = message.getString().getSize();
	auto length = message.getCharacterSize() + message.getLineSpacing();
	auto gpl = bounds.x / length;
	auto num_lines = num_glyphs / gpl;
	for (int i = 0; i < num_lines; ++i) { wrap(); }
	svc::floatReadoutLocator.get() = (float)num_lines;
}

void TextWriter::update() {

	if (!flags.test(MessageState::active)) { return; }
	if (tick_count % writing_speed == 0) {
		char const next_char = (char)message.getString().getData()[glyph_count];
		working_str += next_char;
		working_message.setString(working_str);
		++glyph_count;
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
	message.setString(source[key].as_string().data());

	message.setCharacterSize(text_size);
	message.setFillColor(flcolor::ui_white);
	message.setFont(font);
	message.setPosition(position);
	working_message.setCharacterSize(text_size);
	working_message.setFillColor(flcolor::ui_white);
	working_message.setFont(font);
	working_message.setPosition(position);
}

void TextWriter::write_instant_message(sf::RenderWindow& win) {
	if (!flags.test(MessageState::active)) { return; }
	win.draw(message);
}

void TextWriter::write_gradual_message(sf::RenderWindow& win) {
	if (!flags.test(MessageState::active)) { return; }
	check_if_complete();
	if (complete()) {
		win.draw(message);
		return;
	}
	win.draw(working_message);
}

void TextWriter::check_if_complete() {
	if (glyph_count >= message.getString().getSize()) {
		flags.set(MessageState::complete);
		reset();
	}
}

void TextWriter::reset() {
	writing_speed = default_writing_speed;
	glyph_count = 0;
	tick_count = 0;
	working_message = {};
	working_str = {};
}

void TextWriter::skip_ahead() { writing_speed = fast_writing_speed; }

void TextWriter::activate() { flags.set(MessageState::active); }

void TextWriter::deactivate() {
	flags.reset(MessageState::active);
	flags.reset(MessageState::complete);
	reset();
}

bool text::TextWriter::active() const { return flags.test(MessageState::active); }

bool text::TextWriter::complete() const { return flags.test(MessageState::complete); }

} // namespace text
