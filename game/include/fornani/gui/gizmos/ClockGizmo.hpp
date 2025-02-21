
#pragma once

#include "fornani/graphics/SpriteRotator.hpp"
#include "fornani/gui/Gizmo.hpp"

namespace fornani::gui {

class ClockGizmo : public Gizmo {
  public:
	ClockGizmo(automa::ServiceProvider& svc, world::Map& map);
	void update(automa::ServiceProvider& svc, [[maybe_unused]] player::Player& player, [[maybe_unused]] world::Map& map) override;
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) override;
	bool handle_inputs(config::ControllerMap& controller) override;

  private:
	vfx::SpriteRotator m_rotator{};
	struct {
		sf::Sprite clock;
		sf::Sprite hand;
	} m_sprites;
};

} // namespace fornani::gui
