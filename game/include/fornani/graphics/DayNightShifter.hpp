
#pragma once

#include <SFML/Graphics.hpp>
#include <fornani/io/Logger.hpp>

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani::graphics {

class DayNightShifter {
  public:
	void render(automa::ServiceProvider& svc, sf::RenderTarget& win, sf::Sprite& sprite, int ctr, float temp = -1.f);
	void render(automa::ServiceProvider& svc, sf::RenderTarget& win, sf::Sprite& sprite, int ctr, std::uint8_t const native_alpha, float temp = -1.f);

	[[nodiscard]] auto get_temperature_alpha(int ctr, float temp, std::uint8_t native_alpha = 255) const -> std::optional<std::uint8_t>;

  private:
	io::Logger m_logger{"Graphics"};
};

} // namespace fornani::graphics
