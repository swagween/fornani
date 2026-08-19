
#pragma once

#include <SFML/Graphics.hpp>
#include <fornani/utils/BitFlags.hpp>
#include <fornani/utils/Cooldown.hpp>
#include <fornani/utils/Counter.hpp>

namespace fornani {

namespace automa {
struct ServiceProvider;
}

enum class WorldTimerFlags : std::uint8_t { running, paused };

class WorldTimer {
  public:
	WorldTimer(automa::ServiceProvider& svc);
	void restart();
	void start();
	void pause();
	void resume();
	void update();
	void finish(automa::ServiceProvider& svc);
	void render(sf::RenderWindow& win, sf::Vector2f position);
	void set_tag(std::string_view tag) { m_player_tag = tag; }
	void set_course(int to) { m_course = to; }

	[[nodiscard]] auto is_running() const -> bool { return m_flags.test(WorldTimerFlags::running); }
	[[nodiscard]] auto is_paused() const -> bool { return m_flags.test(WorldTimerFlags::paused); }
	[[nodiscard]] auto get_time() const -> float { return std::clamp(static_cast<float>(m_counter.get_count() * m_ticker->tick_rate), 0.f, std::numeric_limits<float>::max()); }

	void debug();

  private:
	util::Cooldown m_flash{256};
	util::Counter m_counter{};
	util::BitFlags<WorldTimerFlags> m_flags{};
	sf::Text m_text;
	int m_course{};
	util::Ticker* m_ticker;
	std::string m_player_tag{};
};

} // namespace fornani
