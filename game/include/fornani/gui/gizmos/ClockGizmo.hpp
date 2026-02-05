
#pragma once

#include "fornani/graphics/SpriteRotator.hpp"
#include "fornani/gui/Gizmo.hpp"

namespace fornani::gui {

class ClockGizmo : public Gizmo {
  public:
	ClockGizmo(automa::ServiceProvider& svc, world::Map& map, sf::Vector2f placement);
	void update(automa::ServiceProvider& svc, [[maybe_unused]] player::Player& player, [[maybe_unused]] world::Map& map, sf::Vector2f position) override;
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, [[maybe_unused]] player::Player& player, LightShader& shader, Palette& palette, sf::Vector2f cam, bool foreground = false) override;
	bool handle_inputs(input::InputSystem& controller, [[maybe_unused]] audio::Soundboard& soundboard) override;

  private:
	vfx::SpriteRotator m_rotator{};
	sf::Vector2f m_readout_position{};
	struct {
		sf::Sprite clock;
		sf::Sprite hand;
	} m_sprites;
	struct {
		sf::Text readout;
	} m_text;
};

} // namespace fornani::gui
