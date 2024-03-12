
#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Text.hpp>
#include <djson/json.hpp>
#include <array>
#include <deque>
#include <string>
#include <string_view>
#include <xstring>
#include "../utils/BitFlags.hpp"

namespace automa {
struct ServiceProvider;
}

namespace text {

struct Message {
	sf::Text data{};
	bool prompt{};
	int target{};
};

enum class MessageState { writing, selection_mode, cannot_skip };
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
	void load_message(dj::Json& source, std::string_view key);
	void stylize(sf::Text& msg, bool is_suite);
	void write_instant_message(sf::RenderWindow& win);
	void write_gradual_message(sf::RenderWindow& win);
	void write_responses(sf::RenderWindow& win);
	void activate();
	void deactivate();
	void request_next();
	void check_for_prompt(Message& msg);
	void adjust_selection(int amount);
	void process_selection();
	void shutdown();

	void skip_ahead();
	void enable_skip();
	void reset();

	bool writing() const;
	bool complete() const;
	bool selection_mode() const;
	bool can_skip() const;

	Message& const current_message(); //for debug
	Message& const current_response(); // for debug
	int get_current_selection();
	int get_current_suite_set();

	// public for debugging
	int text_size{16};

  private:
	std::deque<std::deque<Message> > suite{};
	std::deque<std::deque<Message> > responses{};

	struct {
		int current_suite_set{};
		int current_response_set{};
		int current_selection{};
	} iterators{};

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

	Message zero_option{}; //for debug
};

} // namespace text
