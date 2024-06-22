
#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Text.hpp>
#include <djson/json.hpp>
#include <array>
#include <deque>
#include <unordered_map>
#include <string>
#include <string_view>
#include "../utils/BitFlags.hpp"
#include "../utils/Shipment.hpp"

namespace automa {
struct ServiceProvider;
}

namespace text {

struct Message {
	sf::Text data{};
	bool prompt{};
	int target{};
};

enum class Codes { prompt, quest, item, voice, emotion };
enum class MessageState { writing, selection_mode, cannot_skip, response_trigger };
static int const default_writing_speed{8};
static int const fast_writing_speed{1};

class TextWriter {
  public:
	TextWriter() = default;
	TextWriter(automa::ServiceProvider& svc);
	void start();
	void update();
	void set_position(sf::Vector2<float> pos);
	void set_bounds(sf::Vector2<float> new_bounds);
	void wrap();
	void load_single_message(std::string_view message);
	void load_message(dj::Json& source, std::string_view key);
	void stylize(sf::Text& msg, bool is_suite) const;
	void write_instant_message(sf::RenderWindow& win);
	void write_gradual_message(sf::RenderWindow& win);
	void write_responses(sf::RenderWindow& win);
	void activate();
	void deactivate();
	void request_next();
	void check_for_event(Message& msg, Codes code);
	void adjust_selection(int amount);
	void process_selection();
	void shutdown();

	void skip_ahead();
	void enable_skip();
	void reset();

	[[nodiscard]] auto writing() const -> bool { return flags.test(MessageState::writing); };
	[[nodiscard]] auto complete() const -> bool { return !flags.test(MessageState::writing) && suite.empty(); };
	[[nodiscard]] auto selection_mode() const -> bool { return flags.test(MessageState::selection_mode); };
	[[nodiscard]] auto can_skip() const -> bool { return !flags.test(MessageState::cannot_skip); };
	[[nodiscard]] auto response_triggered() const -> bool { return flags.test(MessageState::response_trigger); }
	[[nodiscard]] auto responding() const -> bool { return selection_mode(); }
	[[nodiscard]] auto suite_size() const -> size_t { return suite.size(); }

	void reset_response() { flags.reset(MessageState::response_trigger); }
	void flush_communicators() { communicators = {}; }

	Message& current_message();	 // for debug
	Message& current_response(); // for debug
	int get_current_selection() const;
	int get_current_suite_set() const;

	// public for debugging
	int text_size{16};

	struct {
		util::Shipment out_item{};
		util::Shipment out_quest{};
		util::Shipment out_voice{};
		util::Shipment out_emotion{};
	} communicators{};

  private:
	std::deque<std::deque<Message>> suite{};
	std::deque<std::deque<Message>> responses{};

	struct {
		int current_suite_set{};
		int current_response_set{};
		int current_selection{};
	} iterators{};

	std::unordered_map<Codes, char> special_characters{};

	sf::Text working_message{};
	std::string working_str{};
	sf::Font font{};
	int glyph_count{};
	int tick_count{};
	int writing_speed{default_writing_speed};
	util::BitFlags<MessageState> flags{};
	sf::Vector2<float> position{};
	sf::Vector2<float> bounds{};
	sf::Vector2<float> response_position{300.f, 200.f};
	sf::Vector2<float> response_offset{120.f, 60.f};
	float pad{30};

	sf::RectangleShape indicator{};

	automa::ServiceProvider* m_services;

	Message zero_option{}; // for debug
};

} // namespace text
