
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

enum class MessageState { writing };
static int const default_writing_speed{12};
static int const fast_writing_speed{1};

class TextWriter {
  public:
	void start();
	void update();
	void set_position(sf::Vector2<float> pos);
	void set_bounds(sf::Vector2<float> new_bounds);
	void wrap();
	void load_message(automa::ServiceProvider& svc, dj::Json& source, std::string_view key);
	void stylize(sf::Text& msg);
	void write_instant_message(sf::RenderWindow& win);
	void write_gradual_message(sf::RenderWindow& win);
	void activate();
	void deactivate();
	void request_next();

	void skip_ahead();
	void reset();

	bool writing() const;
	bool complete() const;

	// public for debugging
	int text_size{16};

  private:

	std::deque<sf::Text> suite{};

	sf::Text message{};
	sf::Text working_message{};
	std::string working_str{};
	sf::Font font{};
	int glyph_count{};
	int tick_count{};
	int writing_speed{default_writing_speed};
	util::BitFlags<MessageState> flags{};
	sf::Vector2<float> position{};
	sf::Vector2<float> bounds{};
};

} // namespace text
