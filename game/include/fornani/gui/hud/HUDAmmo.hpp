
#pragma once

#include <fornani/gui/hud/HUDWidget.hpp>

namespace fornani::gui {

enum class HUDAmmoFlags { switched };

class HUDAmmo final : public HUDWidget, public Flaggable<HUDAmmoFlags> {
  public:
	HUDAmmo(automa::ServiceProvider& svc, player::Player& player, std::string_view tag, sf::Vector2f root, sf::Vector2i dimensions);

	void update(automa::ServiceProvider& svc, player::Player& player) override;
	void render(automa::ServiceProvider& svc, player::Player& player, sf::RenderWindow& win, sf::Vector2f offset = {}) override;
	[[nodiscard]] auto get_offset(bool scaled = true) const -> sf::Vector2f override;

  private:
	StatusBar m_reload_bar;
	std::optional<WidgetBar> m_ammo_bar{};
};

} // namespace fornani::gui
