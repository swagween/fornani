
#pragma once

#include <SFML/Graphics.hpp>
#include "fornani/graphics/Colors.hpp"
#include "fornani/utils/Cooldown.hpp"
#include "fornani/utils/Counter.hpp"

namespace fornani::player {
class Player;
}

namespace fornani::graphics {

class Transition {

  public:
	Transition(sf::Vector2f screen_dim, int duration, sf::Color color = colors::ui_black);

	void update(player::Player& player);
	void render(sf::RenderWindow& win);
	void start();
	void end();
	[[nodiscard]] auto has_waited(int time) -> bool { return m_hang_time.get_count() >= time; }
	[[nodiscard]] auto not_started() const -> bool { return !m_fade_out && !m_done; }
	[[nodiscard]] auto is_done() const -> bool { return m_done; }

  private:
	util::Counter m_hang_time{};
	util::Cooldown m_cooldown;
	std::uint8_t m_alpha;
	bool m_done{};
	bool m_fade_out{};
	bool m_fade_in{};

	sf::RectangleShape m_box;
	sf::Color m_color;
};

} // namespace fornani::graphics
