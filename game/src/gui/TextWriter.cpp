
#include "fornani/gui/TextWriter.hpp"
#include "fornani/service/ServiceProvider.hpp"

#include <SFML/Graphics.hpp>

#include <string>

namespace fornani::gui {

TextWriter::TextWriter(automa::ServiceProvider& svc)
	: m_services(&svc), help_marker(svc), working_message{svc.text.fonts.basic}, zero_option{.data{svc.text.fonts.basic}}, m_font{&svc.text.fonts.basic}, m_mode{WriterMode::stall}, m_delay{util::Cooldown{32}},
	  m_writing_speed{default_writing_speed_v}, m_delta_threshold{8.f} {
	special_characters.insert({Codes::prompt, '%'});
	special_characters.insert({Codes::quest, '$'});
	special_characters.insert({Codes::item, '^'});
	special_characters.insert({Codes::voice, '&'});
	special_characters.insert({Codes::emotion, '@'});
	special_characters.insert({Codes::hash, '#'});
	help_marker.set_color(svc.styles.colors.ui_white);
	help_marker.set_alpha(0);
	bounds_box.setFillColor(sf::Color(200, 200, 10, 10));
	bounds_box.setOutlineColor(sf::Color(255, 80, 80, 180));
	bounds_box.setOutlineThickness(-1);
	cursor.setFillColor(svc.styles.colors.ui_white);
	cursor.setSize({10.f, 16.f});
	sf::Text testy{svc.text.fonts.basic};
	testy.setString("AAAA");
	NANI_LOG_DEBUG(m_logger, "AAAA: {}", testy.getLocalBounds().size.x);
	testy.setString("AA\nAA");
	NANI_LOG_DEBUG(m_logger, "AA<newln>AA: {}", testy.getLocalBounds().size.x);
}

TextWriter::TextWriter(automa::ServiceProvider& svc, dj::Json& source, std::string_view key) : TextWriter(svc) {
	NANI_LOG_INFO(m_logger, "Json load.");
	load_message(source, key);
}

TextWriter::TextWriter(automa::ServiceProvider& svc, std::string_view message) : TextWriter(svc) {
	NANI_LOG_INFO(m_logger, "Single Message load.");
	load_single_message(message);
}

void TextWriter::start() {

	// to be replaced with something prettier later (maybe)
	indicator.setSize({4.f, 4.f});
	indicator.setOrigin({2.f, 2.f});
	indicator.setFillColor(m_services->styles.colors.bright_orange);

	NANI_LOG_INFO(m_logger, "start() called.");
	if (iterators.current_suite_set >= suite.size()) { return; }
	NANI_LOG_INFO(m_logger, "current suite set iterator is less than size.");
	if (suite.at(iterators.current_suite_set).empty()) { return; }
	NANI_LOG_INFO(m_logger, "current suite set is not empty.");

	working_message = suite.at(iterators.current_suite_set).front().data;
	NANI_LOG_INFO(m_logger, "working message set.");
	constrain();

	m_mode = WriterMode::write;
	m_delay.start();
	NANI_LOG_INFO(m_logger, "start() completed.");
}

void TextWriter::update() {

	bounds_box.setPosition(m_bounds.position);
	bounds_box.setSize(m_bounds.size);
	m_delay.update();

	// delay before next suite
	/*if (flags.test(MessageState::done_writing) && !flags.test(MessageState::started_delay)) {
		delay.start();
		flags.set(MessageState::started_delay);
	}*/
	if (is_writing() && !m_delay.running()) { m_delay.start(); }

	// escape if bounds are invalid
	if (iterators.current_suite_set >= suite.size()) { return; }
	if (suite.at(iterators.current_suite_set).empty()) { shutdown(); }

	if (!is_writing()) { return; }

	// append next character to working string
	if (m_counters.tick.get_count() % m_writing_speed == 0) {
		char const next_char = static_cast<char>(suite.at(iterators.current_suite_set).front().data.getString().getData()[m_counters.glyph.get_count()]);
		working_str += next_char;
		working_message.setString(working_str);
		m_counters.glyph.update();
		m_counters.tick.cancel();
	}
	if (m_counters.glyph.get_count() >= suite.at(iterators.current_suite_set).front().data.getString().getSize()) {
		m_mode = WriterMode::wait;
		reset();
	}
	m_counters.tick.update();
}

void TextWriter::flush() {
	suite.clear();
	working_message.setString("");
	working_str = {};
}

void TextWriter::set_bounds(sf::FloatRect to_bounds) {
	m_bounds = to_bounds;
	if (abs(m_bounds.size.x - m_previous_bounds.size.x) > m_delta_threshold) {
		constrain();
		m_previous_bounds = m_bounds;
	}
}

void TextWriter::wrap() {
	if (iterators.current_suite_set >= suite.size()) { return; }
	if (suite.at(iterators.current_suite_set).empty()) { return; }
	auto& current_message = suite.at(iterators.current_suite_set).front().data;

	// get index of last in-bounds space
	int last_space_index{};
	for (auto i{0}; i < current_message.getString().getSize() - 1; ++i) {
		char const current_char = static_cast<char>(current_message.getString().getData()[i]);
		if (current_char == ' ') {
			if (current_message.findCharacterPos(static_cast<std::size_t>(i + 1)).x > (m_bounds.size.x + m_bounds.position.x)) {
				// splice!
				std::string left = current_message.getString().substring(0, static_cast<std::size_t>(last_space_index + 1));
				std::string right = current_message.getString().substring(static_cast<std::size_t>(last_space_index + 1));
				left += '\n';

				current_message.setString(left + right);
			} else {
				last_space_index = i;
			}
		}
	}
}

void TextWriter::constrain() {
	if (iterators.current_suite_set >= suite.size()) { return; }
	if (suite.at(iterators.current_suite_set).empty()) { return; }
	if (m_bounds.size.x < 64.f) { return; }
	auto& current_message{suite.at(iterators.current_suite_set).front().data};

	// strip out new line characters
	auto new_str{std::string{current_message.getString()}};
	std::erase_if(new_str, [](auto const& c) { return c == '\n'; });
	current_message.setString(new_str);
	NANI_LOG_DEBUG(m_logger, "string to be constrained: {}", new_str);
	wrap();
}

void TextWriter::load_single_message(std::string_view message) {
	flush();
	auto message_container = std::deque<Message>{};
	message_container.push_back({sf::Text(*m_font), false});
	message_container.back().data.setString(message.data());
	stylize(message_container.back().data);
	suite.push_back(message_container);
}

void TextWriter::load_message(dj::Json& source, std::string_view key) {
	flush();

	NANI_LOG_INFO(m_logger, "Flushed.");
	// suite
	for (auto& set : source[key]["suite"].array_view()) {
		auto this_set = std::deque<Message>{};
		for (auto& msg : set.array_view()) {
			this_set.push_back({sf::Text(*m_font), false});
			this_set.back().data.setString(msg.as_string().data());
			stylize(this_set.back().data);
		}
		suite.push_back(this_set);
	}
	NANI_LOG_INFO(m_logger, "Suite loaded.");
	working_message = suite.at(iterators.current_suite_set).front().data;
	help_marker = graphics::HelpText(*m_services, "Press [", config::DigitalAction::menu_select, "] to continue.");
}

void TextWriter::append(std::string_view content) {
	if (suite.empty()) { return; }
	if (suite.back().empty()) { return; }
	auto msg = suite.back().back().data.getString();
	suite.back().back().data.setString(msg + content.data());
}

void TextWriter::stylize(sf::Text& msg) const {
	msg.setCharacterSize(text_size);
	msg.setFillColor(m_services->styles.colors.ui_white);
	msg.setFont(*m_font);
	msg.setLineSpacing(1.5f);
	msg.setPosition(m_bounds.position);
}

void TextWriter::write_instant_message(sf::RenderWindow& win) {
	win.draw(bounds_box);
	if (iterators.current_suite_set >= suite.size()) { return; }
	if (suite.at(iterators.current_suite_set).empty()) { return; }
	bounds_box.setOutlineColor(sf::Color{0, 255, 80});
	bounds_box.setSize(suite.at(iterators.current_suite_set).front().data.getLocalBounds().size);
	win.draw(bounds_box);
	bounds_box.setOutlineColor(sf::Color(255, 80, 80, 180));
	m_mode = WriterMode::wait;
	static bool show_cursor;
	auto& current_message{suite.at(iterators.current_suite_set).front().data};
	working_message = current_message;
	write_gradual_message(win);
}

void TextWriter::write_gradual_message(sf::RenderWindow& win) {
	win.draw(bounds_box);
	static bool show_cursor;
	auto cursor_offset{sf::Vector2f{8.f, 0.f}};
	if (iterators.current_suite_set >= suite.size()) { return; }
	if (suite.at(iterators.current_suite_set).empty()) { return; }
	auto& current_message{suite.at(iterators.current_suite_set).front().data};
	current_message.setPosition(m_bounds.position);
	if (!is_writing()) {
		win.draw(current_message);
		if (m_services->ticker.every_x_frames(24)) { show_cursor = !show_cursor; }
		auto last_glyph_position = current_message.findCharacterPos(working_message.getString().getSize() - 1);
		cursor.setPosition(last_glyph_position + cursor_offset);
		if (show_cursor) { win.draw(cursor); }
		return;
	}
	show_cursor = true;
	help_marker.start();
	working_message.setFillColor(m_services->styles.colors.ui_white);
	working_message.setPosition(m_bounds.position);
	auto last_glyph_position = working_message.findCharacterPos(working_message.getString().getSize() - 1);
	cursor.setPosition(last_glyph_position + cursor_offset);
	win.draw(working_message);
	win.draw(cursor);
}

void TextWriter::reset() {
	m_mode = WriterMode::write;
	m_writing_speed = default_writing_speed_v;
	help_marker.reset();
	m_counters = {};
	working_message.setString("");
	working_str = {};
	iterators.current_selection = 0;
}

void TextWriter::speed_up() { m_writing_speed = fast_writing_speed_v; }

void TextWriter::slow_down() { m_writing_speed = default_writing_speed_v; }

bool TextWriter::request_next() {
	// return true when we are able to progress in the writer
	// return false if inputs should do nothing

	// writer is writing, not ready
	if (is_writing()) { return false; }

	// done writing and no more text suites to write
	if (suite.empty() || iterators.current_suite_set >= suite.size()) {
		shutdown();
		return true;
	}
	if (suite.at(iterators.current_suite_set).empty()) {
		reset();
		return true;
	}
	suite.at(iterators.current_suite_set).pop_front();
	if (suite.at(iterators.current_suite_set).empty()) {
		shutdown();
		return true;
	}
	reset();
	start();

	return true;
}

void TextWriter::shutdown() { m_mode = WriterMode::close; }

Message& TextWriter::current_message() {
	if (iterators.current_suite_set >= suite.size()) { return zero_option; }
	if (suite.at(iterators.current_suite_set).empty()) { return zero_option; }
	return suite.at(iterators.current_suite_set).front();
}

int TextWriter::get_current_selection() const { return iterators.current_selection; }

int TextWriter::get_current_suite_set() const { return iterators.current_suite_set; }

} // namespace fornani::gui
