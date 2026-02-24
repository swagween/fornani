
#include <SFML/Graphics.hpp>
#include <ccmath/ext/clamp.hpp>
#include <fornani/gui/console/TextWriter.hpp>
#include <string>
#include "fornani/service/ServiceProvider.hpp"

namespace fornani::gui {

TextWriter::TextWriter(automa::ServiceProvider& svc)
	: m_services(&svc), working_message{svc.text.fonts.basic}, zero_option{.data{svc.text.fonts.basic}}, m_font{&svc.text.fonts.basic}, m_mode{WriterMode::stall}, m_delay{util::Cooldown{32}}, m_writing_speed{default_writing_speed_v},
	  m_delta_threshold{8.f}, m_text_size{16}, m_input_code{"   "} {
	NANI_LOG_DEBUG(m_logger, "TextWriter ctor @{}", static_cast<void const*>(this));
	bounds_box.setFillColor(sf::Color(200, 200, 10, 10));
	bounds_box.setOutlineColor(sf::Color(255, 80, 80, 180));
	bounds_box.setOutlineThickness(-2.f);
	cursor.setFillColor(colors::ui_white);
	cursor.setSize({10.f, 16.f});
}

TextWriter::TextWriter(automa::ServiceProvider& svc, dj::Json& source) : TextWriter(svc) { load_message(source); }

TextWriter::TextWriter(automa::ServiceProvider& svc, dj::Json& source, std::string_view key, int target_index) : TextWriter(svc) { load_message(source, key, target_index); }

TextWriter::TextWriter(automa::ServiceProvider& svc, std::string_view message) : TextWriter(svc) { load_single_message(message); }

TextWriter::TextWriter(automa::ServiceProvider& svc, std::string_view message, sf::FloatRect bounds) : TextWriter(svc) {
	load_single_message(message);
	set_bounds(bounds, true);
}

void TextWriter::start() {

	if (!suite) { return; }
	if (m_iterators.current_suite_set >= suite->suite.size()) { return; }
	if (suite->suite.at(m_iterators.current_suite_set).empty()) { return; }

	working_message = suite->suite.at(m_iterators.current_suite_set).at(m_iterators.index).data;
	constrain();

	m_mode = WriterMode::write;
	m_delay.start();
	update();
}

void TextWriter::update() {

	bounds_box.setPosition(m_bounds.position);
	bounds_box.setSize(m_bounds.size);
	m_delay.update();

	// delay before next suite
	if (is_writing() && !m_delay.running()) { m_delay.start(); }

	// escape if bounds are invalid
	if (!suite) { return; }
	if (m_iterators.current_suite_set >= suite->suite.size()) { return; }
	if (suite->suite.at(m_iterators.current_suite_set).empty()) { shutdown(); }
	if (is_responding()) {
		working_message = suite->suite.at(m_iterators.current_suite_set).at(m_iterators.index).data;
		return;
	}
	if (!is_writing()) { return; }

	// append next character to working string
	if (m_services->ticker.every_x_ticks(16)) { constrain(); }
	if (m_counters.tick.get_count() % m_writing_speed == 0) {
		char const next_char = static_cast<char>(suite->suite.at(m_iterators.current_suite_set).at(m_iterators.index).data.getString().getData()[m_counters.glyph.get_count()]);
		working_str += next_char;
		working_message.setString(working_str);
		m_counters.glyph.update();
		m_counters.tick.start();
	}
	if (m_counters.glyph.get_count() >= suite->suite.at(m_iterators.current_suite_set).at(m_iterators.index).data.getString().getSize()) { m_mode = WriterMode::wait; }
	m_counters.tick.update();
}

void TextWriter::flush() {
	suite.reset();
	working_message.setString("");
	working_str = {};
}

void TextWriter::wait() { m_mode = WriterMode::wait; }

void TextWriter::respond() { m_mode = WriterMode::respond; }

void TextWriter::stall() { m_mode = WriterMode::stall; }

void TextWriter::debug() {
	ImGui::SetNextWindowPos(ImVec2{20.f, 256.f});
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
	if (!suite) { return; }
	if (m_iterators.current_suite_set >= suite->suite.size()) { return; }
	if (suite->suite.at(m_iterators.current_suite_set).empty()) { return; }
	auto& current_message = suite->suite.at(m_iterators.current_suite_set).at(m_iterators.index).data;
	word_wrap(current_message, m_bounds.position.x + m_bounds.size.x);
}

void TextWriter::constrain() {
	if (!suite) { return; }
	if (m_iterators.current_suite_set >= suite->suite.size()) { return; }
	if (suite->suite.at(m_iterators.current_suite_set).empty()) { return; }
	if (m_bounds.size.x < 64.f) { return; }
	auto& current_message = suite->suite.at(m_iterators.current_suite_set).at(m_iterators.index).data;

	// strip out new line characters
	auto new_str{std::string{current_message.getString()}};
	std::erase_if(new_str, [](auto const& c) { return c == '\n'; });
	current_message.setString(new_str);
	wrap();
}

void TextWriter::load_single_message(std::string_view message) {
	flush();
	auto message_container = std::deque<Message>{};
	message_container.push_back({sf::Text(*m_font)});
	message_container.back().data.setString(message.data());
	stylize(message_container.back().data);
	suite = DialogueSuite{message_container};
	constrain();
}

void TextWriter::load_message(dj::Json& source) {
	flush();
	suite = DialogueSuite{source, *m_font, *m_services, -1};
	for (auto& msg : suite->suite) {
		for (auto& m : msg) { stylize(m.data); }
	}
	working_message = suite->get_message(m_iterators.current_suite_set, m_iterators.index);
	working_message.setString("");
	working_str = {};
	m_is_first = true;
}

void TextWriter::load_message(dj::Json& source, std::string_view key, int target_index) {
	flush();
	suite = DialogueSuite{source, *m_font, *m_services, key, target_index};
	for (auto& msg : suite->suite) {
		for (auto& m : msg) { stylize(m.data); }
	}
	working_message = suite->get_message(m_iterators.current_suite_set, m_iterators.index);
	working_message.setString("");
	working_str = {};
	m_is_first = true;
}

void TextWriter::append(std::string_view content) {
	if (!suite) { return; }
	if (suite->suite.empty()) { return; }
	if (suite->suite.back().empty()) { return; }
	auto msg = suite->suite.back().back().data.getString();
	suite->suite.back().back().data.setString(msg + content.data());
}

void TextWriter::stylize(sf::Text& msg) const {
	msg.setCharacterSize(m_text_size);
	msg.setFillColor(colors::ui_white);
	msg.setFont(*m_font);
	msg.setLineSpacing(1.5f);
	msg.setPosition(m_bounds.position);
}

void TextWriter::set_suite(int to_suite) {
	m_iterators.current_suite_set = ccm::ext::clamp(to_suite, 0, static_cast<int>(suite->suite.size() - 1));
	m_iterators.index = 0;
	reset();
}

void TextWriter::set_index(int to_index) { m_iterators.index = ccm::ext::clamp(to_index, 0, static_cast<int>(suite->suite.at(m_iterators.current_suite_set).size() - 1)); }

void TextWriter::write_instant_message(sf::RenderWindow& win) {
	// win.draw(bounds_box);
	if (!suite) { return; }
	if (m_iterators.current_suite_set >= suite->suite.size()) { return; }
	if (suite->suite.at(m_iterators.current_suite_set).empty()) { return; }
	bounds_box.setOutlineColor(sf::Color{0, 255, 80});
	bounds_box.setSize(suite->suite.at(m_iterators.current_suite_set).at(m_iterators.index).data.getLocalBounds().size);
	// win.draw(bounds_box);
	bounds_box.setOutlineColor(sf::Color(255, 80, 80, 180));
	m_mode = WriterMode::wait;
	auto& current_message{suite->suite.at(m_iterators.current_suite_set).at(m_iterators.index).data};
	working_message = current_message;
	m_hide_cursor = true;
	write_gradual_message(win);
}

void TextWriter::write_gradual_message(sf::RenderWindow& win) {
	// win.draw(bounds_box);
	if (!suite) { return; }
	if (m_mode == WriterMode::stall) { return; }
	static bool show_cursor;
	static auto blink_rate{24};
	auto cursor_offset{sf::Vector2f{8.f, 0.f}};
	if (m_iterators.current_suite_set >= suite->suite.size()) { return; }
	if (suite->suite.at(m_iterators.current_suite_set).empty()) { return; }
	auto& current_message = suite->suite.at(m_iterators.current_suite_set).at(m_iterators.index).data;
	current_message.setPosition(m_bounds.position);
	if (!is_writing()) {
		win.draw(current_message);
		if (m_services->ticker.every_x_frames(blink_rate)) { show_cursor = !show_cursor; }
		auto last_glyph_position = current_message.findCharacterPos(working_message.getString().getSize() - 1);
		cursor.setPosition(last_glyph_position + cursor_offset);
		if (show_cursor && !m_hide_cursor) { win.draw(cursor); }
		// insert an icon hint if there is one
		if (m_flags.test(WriterFlags::input_hint) && m_input_icon) { insert_input_hint(win, current_message); }
		return;
	}
	show_cursor = true;
	working_message.setFillColor(colors::ui_white);
	working_message.setPosition(m_bounds.position);
	auto last_glyph_position = working_message.findCharacterPos(working_message.getString().getSize() - 1);
	cursor.setPosition(last_glyph_position + cursor_offset);
	win.draw(working_message);
	if (!m_hide_cursor) { win.draw(cursor); }
	if (m_flags.test(WriterFlags::input_hint) && m_input_icon) { insert_input_hint(win, working_message); }
}

void TextWriter::insert_input_hint(sf::RenderWindow& win, sf::Text& message) {
	auto insertion_index = message.getString().find(m_input_code);
	if (insertion_index == std::string::npos) { return; }
	auto insertion_point = message.findCharacterPos(insertion_index);
	m_input_icon->setPosition(insertion_point);
	win.draw(*m_input_icon);
}

void TextWriter::set_font_color(sf::Color to_color) {
	if (!suite) { return; }
	if (suite->suite.empty()) { return; }
	for (auto& msg : suite->suite.back()) { msg.data.setFillColor(to_color); }
	cursor.setFillColor(to_color);
}

void TextWriter::set_font(sf::Font& to_font) { m_font = &to_font; }

void TextWriter::reset() {
	slow_down();
	m_counters.glyph.start();
	working_str = {};
	m_mode = WriterMode::write;
}

void TextWriter::speed_up() { m_writing_speed = fast_writing_speed_v; }

void TextWriter::slow_down() { m_writing_speed = default_writing_speed_v; }

void TextWriter::insert_icon_at(int index, sf::Vector2i icon_lookup) {
	if (!suite) { return; }
	if (m_iterators.current_suite_set >= suite->suite.size()) { return; }
	if (suite->suite.at(m_iterators.current_suite_set).empty()) { return; }
	auto& current_message = suite->suite.at(m_iterators.current_suite_set).at(m_iterators.index).data;
	if (m_input_icon) { return; } // don't do this if we've already done it last tick
	m_input_icon = sf::Sprite{m_services->assets.get_texture("controller_button_icons")};
	m_input_icon->setTextureRect(sf::IntRect{icon_lookup * 18, {18, 18}});
	m_input_icon->setScale(constants::f_scale_vec);
	m_input_icon->setOrigin({-2.f, 4.f});
	auto msg = current_message.getString();
	auto first = msg.substring(0, index);
	auto second = msg.substring(index, std::string::npos);
	current_message.setString(first + m_input_code + second);
	m_flags.set(WriterFlags::input_hint);
}

bool TextWriter::request_next() {
	// writer is writing, not ready
	if (is_writing()) { return false; }
	++m_iterators.index;
	m_is_first = false;
	reset();
	if (!suite) { return false; }
	if (m_iterators.index >= suite->suite.at(m_iterators.current_suite_set).size()) { shutdown(); }
	return true;
}

void TextWriter::shutdown() { m_mode = WriterMode::close; }

Message& TextWriter::current_message() {
	if (!suite) { return zero_option; }
	if (m_iterators.current_suite_set >= suite->suite.size()) { return zero_option; }
	if (suite->suite.at(m_iterators.current_suite_set).empty()) { return zero_option; }
	return suite->suite.at(m_iterators.current_suite_set).at(m_iterators.index);
}

DialogueSuite::DialogueSuite(dj::Json const& in, sf::Font& font, automa::ServiceProvider& svc, int target_index) {
	NANI_LOG_DEBUG(m_logger, "DialogueSuite ctor @{} suite size={}", static_cast<void const*>(this), in["suite"].is_array() ? in["suite"].as_array().size() : -1);
	for (auto const& set : in["suite"].as_array()) {
		auto this_set = std::deque<Message>{};
		for (auto const& msg : set.as_array()) {
			if (msg["contingencies"]) {
				auto contingencies = std::vector<QuestContingency>{};
				for (auto const& cont : msg["contingencies"].as_array()) { contingencies.push_back(QuestContingency{cont}); }
				NANI_LOG_DEBUG(m_logger, "Quest Contingency detected.");
				if (!svc.quest_table.are_contingencies_met(contingencies)) { continue; }
			}

			auto codes = std::vector<MessageCode>{};
			auto which = 0;
			if (msg["messages"].is_array()) { which = target_index == -1 ? random::random_range(0, msg["messages"].as_array().size() - 1) : target_index; }
			if (msg["codes"].is_array()) {
				for (auto const& code : msg["codes"].as_array()) {
					codes.push_back(MessageCode{code});
					// NANI_LOG_DEBUG(m_logger, "Code pushed to this set: {}", codes.back().value);
				}
			}
			if (msg["code_list"].is_array()) {
				for (auto const& code : msg["code_list"][which].as_array()) {
					codes.push_back(MessageCode{code});
					// NANI_LOG_DEBUG(m_logger, "Code pushed to this set: {}", codes.back().value);
				}
			}
			this_set.push_back({sf::Text(font), codes});
			if (msg["message"]) {
				this_set.back().data.setString(msg["message"].as_string().data());
				// NANI_LOG_DEBUG(m_logger, "Message pushed to this set: {}", std::string{msg["message"].as_string()});
			}
			if (msg["messages"].is_array()) { this_set.back().data.setString(msg["messages"][which].as_string().data()); }
		}
		// NANI_LOG_DEBUG(m_logger, "Finished set: {} messages kept", this_set.size());
		if (!this_set.empty()) { suite.push_back(std::move(this_set)); }
	}
	if (suite.empty()) { NANI_LOG_DEBUG(m_logger, "Invalid suite!"); }
}

DialogueSuite::DialogueSuite(dj::Json const& in, sf::Font& font, automa::ServiceProvider& svc, std::string_view key, int target_index) : DialogueSuite(in[key], font, svc, target_index) {}

} // namespace fornani::gui
