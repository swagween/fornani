#pragma once

#include <SFML/Graphics.hpp>
#include <string_view>
#include "../utils/BitFlags.hpp"
#include "../utils/Cooldown.hpp"
#include "../utils/Counter.hpp"

namespace automa {
struct ServiceProvider;
}

namespace config {
enum class DigitalAction;
}

namespace text {

enum class HelpTextFlags { no_blink, time_limit };

class HelpText {
  public:
	void init(automa::ServiceProvider& svc, std::string start, config::DigitalAction const& code, std::string end = "", int delay_time = 195, bool include_background = false, bool no_blink = false);
	void render(sf::RenderWindow& win);
	void set_color(sf::Color color);
	void set_string(std::string string);
	void set_alpha(uint8_t a);
	void set_position(sf::Vector2<float> pos);
	void start(int time = 195) { delay.start(time); }
	void reset();
	[[nodiscard]] auto ready() const -> bool { return delay.is_complete(); }
	sf::Text& text() { return data; };

  private:
	util::BitFlags<HelpTextFlags> flags{};
	util::Cooldown delay{195};
	util::Counter alpha_counter{};
	sf::Text data{};
	std::string marker{};
	int text_size{16};
	sf::Color text_color{};
	sf::Color bg_color{};
	sf::Font font{};
	float pad{30};
	bool background{};
	sf::Vector2<float> position{};
	sf::Vector2<float> bg_offset{2.f, 2.f};
};

} // namespace text
