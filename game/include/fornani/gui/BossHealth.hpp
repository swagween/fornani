
#pragma once

#include <SFML/Graphics.hpp>
#include <fornani/gui/HealthBar.hpp>
#include <fornani/utils/RectPath.hpp>
#include <string_view>

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani::gui {

class BossHealth : public HealthBar {
  public:
	BossHealth(automa::ServiceProvider& svc, std::string_view label);
	void update(float const percentage);
	void render(sf::RenderWindow& win);
	void bring_in() { m_path.set_section("in"); }
	void send_out() { m_path.set_section("out"); }

  private:
	sf::Text m_label;
	util::RectPath m_path;
};

} // namespace fornani::gui
