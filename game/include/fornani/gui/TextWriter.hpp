
#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Text.hpp>
#include <djson/json.hpp>
#include <deque>
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

struct Message {
	sf::Text data;
	bool prompt{};
	int target{};
};

enum class Codes : std::uint8_t { prompt, quest, item, voice, emotion, hash };
enum class WriterMode : std::uint8_t { write, wait, close, stall, respond };
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
	///@return true when we are able to progress in the writer, false if inputs should do nothing
	bool request_next();
	void speed_up();
	void slow_down();

	[[nodiscard]] auto is_responding() const -> bool { return m_mode == WriterMode::respond; }
	[[nodiscard]] auto is_writing() const -> bool { return m_mode == WriterMode::write; }
	[[nodiscard]] auto is_waiting() const -> bool { return m_mode == WriterMode::wait; }
	[[nodiscard]] auto is_stalling() const -> bool { return m_mode == WriterMode::stall; }
	[[nodiscard]] auto exit_requested() const -> bool { return m_mode == WriterMode::close; }
	[[nodiscard]] auto is_ready() const -> bool { return is_waiting() && m_delay.is_complete(); }

	Message& current_message(); // for debug

	[[nodiscard]] auto get_current_suite_set() const -> int { return m_iterators.current_suite_set; }
	[[nodiscard]] auto get_index() const -> int { return m_iterators.index; }

  private:
	explicit TextWriter(automa::ServiceProvider& svc);
	void load_single_message(std::string_view message);
	void load_message(dj::Json& source, std::string_view key);
	void write_instant_message(sf::RenderWindow& win);
	void write_gradual_message(sf::RenderWindow& win);
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

	sf::RectangleShape indicator{};
	sf::RectangleShape cursor{};

	automa::ServiceProvider* m_services;

	Message zero_option; // for debug
	sf::RectangleShape bounds_box{};

	io::Logger m_logger{"gui"};
};

} // namespace fornani::gui
