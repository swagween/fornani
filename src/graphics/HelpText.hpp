#pragma once

#include <SFML/Graphics.hpp>
#include <string_view>
#include "../utils/Cooldown.hpp"
#include "../utils/Counter.hpp"

namespace automa {
struct ServiceProvider;
}

namespace text {

class HelpText {
  public:
	void init(automa::ServiceProvider& svc, std::string start, std::string_view code = "", std::string end = "");
	void render(sf::RenderWindow& win);
	void set_color(sf::Color color);
	void set_string(std::string string);
	void set_alpha(uint8_t a);
	void set_position(sf::Vector2<float> pos);
	void start() { delay.start(); }
	void reset();
	[[nodiscard]] auto ready() const -> bool { return delay.is_complete(); }
	sf::Text& text() { return data; };

  private:
	util::Cooldown delay{195};
	util::Counter alpha_counter{};
	sf::Text data{};
	std::string marker{};
	int text_size{16};
	sf::Color text_color{};
	sf::Font font{};
	float pad{30};
};

} // namespace flfx
