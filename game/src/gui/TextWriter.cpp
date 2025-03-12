
#include "fornani/gui/TextWriter.hpp"
#include "fornani/service/ServiceProvider.hpp"

#include <SFML/Graphics.hpp>

#include <string>

namespace fornani::gui {

TextWriter::TextWriter(automa::ServiceProvider& svc)
	: m_services(&svc), working_message{svc.text.fonts.basic}, zero_option{.data{svc.text.fonts.basic}}, m_font{&svc.text.fonts.basic}, m_mode{WriterMode::stall}, m_delay{util::Cooldown{32}}, m_writing_speed{default_writing_speed_v},
	  m_delta_threshold{8.f}, m_text_size{16} {
	bounds_box.setFillColor(sf::Color(200, 200, 10, 10));
	bounds_box.setOutlineColor(sf::Color(255, 80, 80, 180));
	bounds_box.setOutlineThickness(-1);
	cursor.setFillColor(svc.styles.colors.ui_white);
	cursor.setSize({10.f, 16.f});
}

TextWriter::TextWriter(automa::ServiceProvider& svc, dj::Json& source, std::string_view key) : TextWriter(svc) { load_message(source, key); }

TextWriter::TextWriter(automa::ServiceProvider& svc, std::string_view message) : TextWriter(svc) { load_single_message(message); }

TextWriter::TextWriter(automa::ServiceProvider& svc, std::string_view message, sf::FloatRect bounds) : TextWriter(svc) {
	load_single_message(message);
	set_bounds(bounds, true);
}

void TextWriter::start() {

	// to be replaced with something prettier later (maybe)
	indicator.setSize({4.f, 4.f});
	indicator.setOrigin({2.f, 2.f});
	indicator.setFillColor(m_services->styles.colors.bright_orange);

	if (m_iterators.current_suite_set >= suite.size()) { return; }
	if (suite.at(m_iterators.current_suite_set).empty()) { return; }

	working_message = suite.at(m_iterators.current_suite_set).at(m_iterators.index).data;
	constrain();

	m_mode = WriterMode::write;
	m_delay.start();
	NANI_LOG_DEBUG(m_logger, "Writer started.");
}

void TextWriter::update() {

	bounds_box.setPosition(m_bounds.position);
	bounds_box.setSize(m_bounds.size);
	m_delay.update();

	// delay before next suite
	if (is_writing() && !m_delay.running()) { m_delay.start(); }

	// escape if bounds are invalid
	if (m_iterators.current_suite_set >= suite.size()) { return; }
	if (suite.at(m_iterators.current_suite_set).empty()) { shutdown(); }
	if (is_responding()) {
		working_message = suite.at(m_iterators.current_suite_set).at(m_iterators.index).data;
		return;
	}
	if (!is_writing()) { return; }

	// append next character to working string
	if (m_counters.tick.get_count() % m_writing_speed == 0) {
		char const next_char = static_cast<char>(suite.at(m_iterators.current_suite_set).at(m_iterators.index).data.getString().getData()[m_counters.glyph.get_count()]);
		working_str += next_char;
		working_message.setString(working_str);
		m_counters.glyph.update();
		m_counters.tick.start();
	}
	if (m_counters.glyph.get_count() >= suite.at(m_iterators.current_suite_set).at(m_iterators.index).data.getString().getSize()) { m_mode = WriterMode::wait; }
	m_counters.tick.update();
}

void TextWriter::flush() {
	suite.clear();
	working_message.setString("");
	working_str = {};
}

void TextWriter::respond() { m_mode = WriterMode::respond; }

void TextWriter::stall() { m_mode = WriterMode::stall; }

void TextWriter::debug() {
	ImGui::SetNextWindowPos(ImVec2{20.f, 128.f});
	ImGui::SetNextWindowSize(ImVec2{256.f, 128.f});
	if (ImGui::Begin("Writer Debug")) {
		ImGui::Text("Mode %s", m_mode == WriterMode::write ? "write" : m_mode == WriterMode::wait ? "wait" : m_mode == WriterMode::respond ? "respond" : m_mode == WriterMode::stall ? "stall" : "close");
		ImGui::Text("Set: %i", m_iterators.current_suite_set);
		ImGui::Text("Index: %i", m_iterators.index);
		ImGui::End();
	}
}

void TextWriter::set_bounds(sf::FloatRect to_bounds, bool wrap) {
	m_bounds = to_bounds;
	if (ccm::abs(m_bounds.size.x - m_previous_bounds.size.x) > m_delta_threshold || wrap) {
		constrain();
		m_previous_bounds = m_bounds;
	}
	bounds_box.setPosition(m_bounds.position);
	bounds_box.setSize(m_bounds.size);
}

void TextWriter::wrap() {
	if (m_iterators.current_suite_set >= suite.size()) { return; }
	if (suite.at(m_iterators.current_suite_set).empty()) { return; }
	auto& current_message = suite.at(m_iterators.current_suite_set).at(m_iterators.index).data;
	int last_space_index{};
	for (auto i{0}; i < current_message.getString().getSize() - 1; ++i) {
		char const current_char = static_cast<char>(current_message.getString().getData()[i]);
		if (current_char == ' ') {
			last_space_index = i;
			std::string left = current_message.getString().substring(0, static_cast<std::size_t>(last_space_index + 1));
			std::string right = current_message.getString().substring(static_cast<std::size_t>(last_space_index + 1));
			auto next_space{std::distance(right.begin(), std::find_if(right.begin(), right.end(), [](auto const& c) { return c == ' '; }))};
			auto next_word = current_message.findCharacterPos(static_cast<std::size_t>(i + next_space));
			if (next_word.x > m_bounds.position.x + m_bounds.size.x) {
				// splice!
				left += '\n';
				current_message.setString(left + right);
			}
		}
	}
}

void TextWriter::constrain() {
	if (m_iterators.current_suite_set >= suite.size()) { return; }
	if (suite.at(m_iterators.current_suite_set).empty()) { return; }
	if (m_bounds.size.x < 64.f) { return; }
	auto& current_message = suite.at(m_iterators.current_suite_set).at(m_iterators.index).data;

	// strip out new line characters
	auto new_str{std::string{current_message.getString()}};
	std::erase_if(new_str, [](auto const& c) { return c == '\n'; });
	current_message.setString(new_str);
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
	working_message = suite.at(m_iterators.current_suite_set).at(m_iterators.index).data;
}

void TextWriter::append(std::string_view content) {
	if (suite.empty()) { return; }
	if (suite.back().empty()) { return; }
	auto msg = suite.back().back().data.getString();
	suite.back().back().data.setString(msg + content.data());
}

void TextWriter::stylize(sf::Text& msg) const {
	msg.setCharacterSize(m_text_size);
	msg.setFillColor(m_services->styles.colors.ui_white);
	msg.setFont(*m_font);
	msg.setLineSpacing(1.5f);
	msg.setPosition(m_bounds.position);
}

void TextWriter::write_instant_message(sf::RenderWindow& win) {
	// win.draw(bounds_box);
	if (m_iterators.current_suite_set >= suite.size()) { return; }
	if (suite.at(m_iterators.current_suite_set).empty()) { return; }
	bounds_box.setOutlineColor(sf::Color{0, 255, 80});
	bounds_box.setSize(suite.at(m_iterators.current_suite_set).at(m_iterators.index).data.getLocalBounds().size);
	// win.draw(bounds_box);
	bounds_box.setOutlineColor(sf::Color(255, 80, 80, 180));
	m_mode = WriterMode::wait;
	auto& current_message{suite.at(m_iterators.current_suite_set).at(m_iterators.index).data};
	working_message = current_message;
	write_gradual_message(win);
}

void TextWriter::write_gradual_message(sf::RenderWindow& win) {
	// win.draw(bounds_box);
	static bool show_cursor;
	auto cursor_offset{sf::Vector2f{8.f, 0.f}};
	if (m_iterators.current_suite_set >= suite.size()) { return; }
	if (suite.at(m_iterators.current_suite_set).empty()) { return; }
	auto& current_message = suite.at(m_iterators.current_suite_set).at(m_iterators.index).data;
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
	working_message.setFillColor(m_services->styles.colors.ui_white);
	working_message.setPosition(m_bounds.position);
	auto last_glyph_position = working_message.findCharacterPos(working_message.getString().getSize() - 1);
	cursor.setPosition(last_glyph_position + cursor_offset);
	win.draw(working_message);
	win.draw(cursor);
}

void TextWriter::set_font_color(sf::Color to_color) {
	if (suite.empty()) { return; }
	for (auto& msg : suite.back()) { msg.data.setFillColor(to_color); }
	cursor.setFillColor(to_color);
}

void TextWriter::reset() {
	slow_down();
	m_counters.glyph.start();
	working_str = {};
	m_mode = WriterMode::write;
	NANI_LOG_DEBUG(m_logger, "Writer reset.");
}

void TextWriter::speed_up() { m_writing_speed = fast_writing_speed_v; }

void TextWriter::slow_down() { m_writing_speed = default_writing_speed_v; }

bool TextWriter::request_next() {
	// writer is writing, not ready
	if (is_writing()) { return false; }
	++m_iterators.index;
	reset();
	if (m_iterators.index >= suite.at(m_iterators.current_suite_set).size()) { shutdown(); }
	return true;
}

void TextWriter::shutdown() { m_mode = WriterMode::close; }

Message& TextWriter::current_message() {
	if (m_iterators.current_suite_set >= suite.size()) { return zero_option; }
	if (suite.at(m_iterators.current_suite_set).empty()) { return zero_option; }
	return suite.at(m_iterators.current_suite_set).at(m_iterators.index);
}

} // namespace fornani::gui
