
#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Text.hpp>
#include <djson/json.hpp>
#include <string>
#include <string_view>
#include <xstring>
#include <array>
#include "../utils/BitFlags.hpp"

namespace text {

	enum class MessageState{ active, complete };
static int const default_writing_speed{8};
static int const fast_writing_speed{2};

class TextWriter {
  public:
	void update();
	void set_position(sf::Vector2<float> pos);
	void set_bounds(sf::Vector2<float> new_bounds);
	void wrap();
	void load_message(dj::Json& source, std::string_view key);
	void write_instant_message(sf::RenderWindow& win);
	void write_gradual_message(sf::RenderWindow& win);
	void check_if_complete();
	void activate();
	void deactivate();

	void skip_ahead();
	void reset();

	bool active() const;
	bool complete() const;

	//public for debugging
	int text_size{24};

  private:
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
