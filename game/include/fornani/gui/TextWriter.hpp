
#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Text.hpp>
#include <djson/json.hpp>
#include <fornani/core/Common.hpp>
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

struct MessageCode {
	MessageCode(dj::Json const& in);
	MessageCodeType type{};
	int value{};
	std::optional<std::vector<int>> extras{};

	void debug();

	[[nodiscard]] auto is_exit() const -> bool { return type == MessageCodeType::exit; }
	[[nodiscard]] auto is_response() const -> bool { return type == MessageCodeType::response; }
	[[nodiscard]] auto is_redirect() const -> bool { return type == MessageCodeType::redirect; }
	[[nodiscard]] auto is_action() const -> bool { return type == MessageCodeType::action; }
	[[nodiscard]] auto is_item() const -> bool { return type == MessageCodeType::item; }
	[[nodiscard]] auto is_destructible() const -> bool { return type == MessageCodeType::destructible; }
	[[nodiscard]] auto is_input_hint() const -> bool { return type == MessageCodeType::input_hint; }
	[[nodiscard]] auto is_reveal_item() const -> bool { return type == MessageCodeType::reveal_item; }
	[[nodiscard]] auto is_start_battle() const -> bool { return type == MessageCodeType::start_battle; }
	[[nodiscard]] auto is_voice_cue() const -> bool { return type == MessageCodeType::voice; }
	[[nodiscard]] auto is_emotion() const -> bool { return type == MessageCodeType::emotion; }
	[[nodiscard]] auto is_pop_conversation() const -> bool { return type == MessageCodeType::pop_conversation; }
	[[nodiscard]] auto is_play_song() const -> bool { return type == MessageCodeType::play_song; }

	// editor helpers
	void mark_for_deletion() { delete_me = true; }
	[[nodiscard]] auto is_marked_for_deletion() const -> bool { return delete_me; }
	bool delete_me{};
};

struct Message {
	sf::Text data;
	bool prompt{};
	std::optional<std::vector<MessageCode>> codes{};
};

enum class Codes : std::uint8_t { prompt, quest, item, voice, emotion, hash };
enum class WriterMode : std::uint8_t { write, wait, close, stall, respond };
enum class WriterFlags : std::uint8_t { input_hint };
static constexpr int default_writing_speed_v{8};
static constexpr int fast_writing_speed_v{1};

class TextWriter {
  public:
	friend class Console;
	friend class DescriptionGizmo;
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

  private:
	explicit TextWriter(automa::ServiceProvider& svc);
	void load_single_message(std::string_view message);
	void load_message(dj::Json& source, std::string_view key);
	void write_instant_message(sf::RenderWindow& win);
	void write_gradual_message(sf::RenderWindow& win);
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

} // namespace fornani::gui
