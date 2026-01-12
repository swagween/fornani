
#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Text.hpp>
#include <djson/json.hpp>
#include <fornani/core/Common.hpp>
#include <fornani/gui/console/Message.hpp>
#include <deque>
#include <ranges>
#include <string>
#include <string_view>
#include <unordered_map>
#include "fornani/graphics/HelpText.hpp"
#include "fornani/io/Logger.hpp"
#include "fornani/utils/BitFlags.hpp"
#include "fornani/utils/Cooldown.hpp"
#include "fornani/utils/Decoder.hpp"
#include "fornani/utils/QuestCode.hpp"
#include "fornani/utils/Shipment.hpp"

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani::gui {

enum class Codes { prompt, quest, item, voice, emotion, hash };
enum class WriterMode { write, wait, close, stall, respond };
enum class WriterFlags { input_hint };
static constexpr int default_writing_speed_v{8};
static constexpr int medium_writing_speed_v{6};
static constexpr int fast_writing_speed_v{1};

class TextWriter {
  public:
	friend class Console;
	friend class DescriptionGizmo;
	explicit TextWriter(automa::ServiceProvider& svc, dj::Json& source);
	explicit TextWriter(automa::ServiceProvider& svc, dj::Json& source, std::string_view key);
	explicit TextWriter(automa::ServiceProvider& svc, std::string_view message);
	explicit TextWriter(automa::ServiceProvider& svc, std::string_view message, sf::FloatRect bounds);
	void start();
	void update();
	void set_bounds(sf::FloatRect to_bounds, bool wrap = false);
	void append(std::string_view content);
	void set_font_color(sf::Color to_color);
	void set_font(sf::Font& to_font);
	///@return true when we are able to progress in the writer, false if inputs should do nothing
	bool request_next();
	void speed_up();
	void slow_down();
	void insert_icon_at(int index, sf::Vector2i icon_lookup);

	[[nodiscard]] auto is_responding() const -> bool { return m_mode == WriterMode::respond; }
	[[nodiscard]] auto is_writing() const -> bool { return m_mode == WriterMode::write; }
	[[nodiscard]] auto is_waiting() const -> bool { return m_mode == WriterMode::wait; }
	[[nodiscard]] auto is_stalling() const -> bool { return m_mode == WriterMode::stall; }
	[[nodiscard]] auto exit_requested() const -> bool { return m_mode == WriterMode::close; }
	[[nodiscard]] auto is_ready() const -> bool { return is_waiting() && m_delay.is_complete(); }
	[[nodiscard]] auto is_available() const -> bool { return !is_writing() && !is_stalling(); }
	[[nodiscard]] auto is_first_message() const -> bool { return m_is_first; }

	Message& current_message(); // for debug

	[[nodiscard]] auto get_current_suite_set() const -> int { return m_iterators.current_suite_set; }
	[[nodiscard]] auto get_index() const -> int { return m_iterators.index; }

	void write_instant_message(sf::RenderWindow& win);
	void write_gradual_message(sf::RenderWindow& win);

  private:
	explicit TextWriter(automa::ServiceProvider& svc);
	void load_single_message(std::string_view message);
	void load_message(dj::Json& source, std::string_view key);
	void load_message(dj::Json& source);
	void insert_input_hint(sf::RenderWindow& win, sf::Text& message);
	void stylize(sf::Text& msg) const;
	void set_suite(int to_suite);
	void set_index(int to_index);
	void wrap();
	void constrain();
	void shutdown();
	void reset();
	void flush();
	void wait();
	void respond();
	void stall();

	void debug();

	WriterMode m_mode{};
	util::Cooldown m_delay;

	std::deque<std::deque<Message>> suite{};

	util::BitFlags<WriterFlags> m_flags{};

	struct {
		int current_suite_set{};
		int index{};
	} m_iterators{};

	struct {
		util::Counter glyph{};
		util::Counter tick{};
	} m_counters{};

	sf::Text working_message;

	std::string working_str{};
	sf::Font* m_font;
	sf::FloatRect m_bounds{};
	sf::FloatRect m_previous_bounds{};
	float m_delta_threshold{};
	int m_writing_speed{};
	int m_text_size{};
	bool m_hide_cursor{};
	bool m_is_first{};

	std::string m_input_code;

	std::optional<sf::Sprite> m_input_icon{};

	sf::RectangleShape cursor{};

	automa::ServiceProvider* m_services;

	Message zero_option; // for debug
	sf::RectangleShape bounds_box{};

	io::Logger m_logger{"gui"};
};

static inline void word_wrap(sf::Text& current_message, float const width) {
	int last_space_index{};
	for (auto i{0}; i < current_message.getString().getSize() - 1; ++i) {
		char const current_char = static_cast<char>(current_message.getString().getData()[i]);
		if (current_char == ' ') {
			last_space_index = i;
			if (last_space_index >= current_message.getString().getSize()) { return; }
			std::string left = current_message.getString().substring(0, static_cast<std::size_t>(last_space_index + 1));
			std::string right = current_message.getString().substring(static_cast<std::size_t>(last_space_index + 1));
			auto next_space{std::distance(right.begin(), std::find_if(right.begin(), right.end(), [](auto const& c) { return c == ' '; }))};
			auto next_word = current_message.findCharacterPos(static_cast<std::size_t>(i + next_space));
			if (next_word.x > width) {
				// splice!
				left += '\n';
				current_message.setString(left + right);
			}
		}
	}
}

} // namespace fornani::gui
