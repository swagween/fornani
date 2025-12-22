
#pragma once

#include <SFML/Graphics.hpp>
#include <fornani/utils/Cooldown.hpp>
#include <fornani/utils/RectPath.hpp>
#include <string_view>

namespace fornani {

namespace automa {
struct ServiceProvider;
}

class Notification {
  public:
	Notification(automa::ServiceProvider& svc, std::string_view message);
	void update(automa::ServiceProvider& svc);
	void render(sf::RenderWindow& win, float y_offset);

	[[nodiscard]] auto is_complete() const -> bool { return m_fade.is_almost_complete(); }

  private:
	sf::Text m_message;
	util::RectPath m_path;
	util::Cooldown m_stall;
	util::Cooldown m_fade;
};

} // namespace fornani
