
#pragma once

#include <SFML/Graphics.hpp>
#include <fornani/utils/BitFlags.hpp>
#include <fornani/utils/Cooldown.hpp>
#include <fornani/utils/Stopwatch.hpp>

namespace fornani {

namespace automa {
struct ServiceProvider;
}

enum class WorldTimerFlags : std::uint8_t { running };

class WorldTimer {
  public:
	WorldTimer(automa::ServiceProvider& svc);
	void start();
	void finish(automa::ServiceProvider& svc);
	void render(sf::RenderWindow& win, sf::Vector2f position);
	void set_tag(std::string_view tag) { m_player_tag = tag; }
	void set_course(int to) { m_course = to; }

	[[nodiscard]] auto is_running() const -> bool { return m_flags.test(WorldTimerFlags::running); }

  private:
	util::Cooldown m_flash{256};
	util::BitFlags<WorldTimerFlags> m_flags{};
	util::Stopwatch m_timer{};
	sf::Text m_text;
	int m_course{};
	std::string m_player_tag{};
};

} // namespace fornani
