
#pragma once

#include <fornani/graphics/Drawable.hpp>
#include <fornani/gui/Gizmo.hpp>

namespace fornani::gui {

class HotbarGizmo : public Gizmo {
  public:
	HotbarGizmo(automa::ServiceProvider& svc, world::Map& map, sf::Vector2f placement);
	void update(automa::ServiceProvider& svc, [[maybe_unused]] player::Player& player, [[maybe_unused]] world::Map& map, sf::Vector2f position) override;
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, [[maybe_unused]] player::Player& player, LightShader& shader, Palette& palette, sf::Vector2f cam, bool foreground = false) override;

	void set_hovered_gun(int to) { m_hovered_gun = to; }

  private:
	Drawable m_screen_sprite;
	sf::RenderTexture m_screen{};
	Drawable m_sprite;
	util::RectPath m_bounds;
	sf::Vector2f m_outline_offset{};
	Animatable m_gun_display;
	int m_hovered_gun{};
};

} // namespace fornani::gui
