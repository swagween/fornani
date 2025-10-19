
#pragma once

#include <SFML/Graphics.hpp>
#include <fornani/graphics/Colors.hpp>
#include <fornani/io/Logger.hpp>
#include <fornani/utils/Cooldown.hpp>
#include <fornani/utils/Counter.hpp>

namespace fornani::player {
class Player;
}

namespace fornani::graphics {

enum class TransitionState : std::uint8_t { inactive, fading_to_black, black, fading_out };

class Transition {

  public:
	Transition(sf::Vector2f screen_dim, int duration, sf::Color color = colors::ui_black);
	void update(player::Player& player);
	void render(sf::RenderWindow& win);
	void start();
	void end();

	[[nodiscard]] auto has_waited(int time) -> bool { return m_hang_time.get_count() >= time; }
	[[nodiscard]] auto is(TransitionState test) const -> bool { return m_state == test; }
	[[nodiscard]] auto get_cooldown() const -> float { return m_cooldown.get_normalized(); }
	[[nodiscard]] auto as_string() const -> std::string;

  private:
	util::Counter m_hang_time{};
	util::Cooldown m_cooldown;
	std::uint8_t m_alpha;

	TransitionState m_state{};

	sf::RectangleShape m_box;
	sf::Color m_color;

	io::Logger m_logger{"Graphics"};
};

} // namespace fornani::graphics
