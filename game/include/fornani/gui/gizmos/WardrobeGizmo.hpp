
#pragma once

#include "fornani/entities/animation/AnimatedSprite.hpp"
#include "fornani/gui/Gizmo.hpp"
#include "fornani/gui/WardrobeWidget.hpp"

namespace fornani::gui {

struct HealthDisplay {
	sf::Sprite hearts;
	sf::Sprite sockets;
	sf::Vector2f position{};
	int socket_state{};
};

class WardrobeGizmo : public Gizmo {
  public:
	WardrobeGizmo(automa::ServiceProvider& svc, world::Map& map, sf::Vector2f placement);
	void update(automa::ServiceProvider& svc, [[maybe_unused]] player::Player& player, [[maybe_unused]] world::Map& map, sf::Vector2f position) override;
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, [[maybe_unused]] player::Player& player, sf::Vector2f cam, bool foreground = false) override;
	bool handle_inputs(config::ControllerMap& controller, [[maybe_unused]] audio::Soundboard& soundboard) override;

  private:
	WardrobeWidget m_nani;
	util::RectPath m_path;
	bool m_wardrobe_update{};
	sf::Vector2f m_nani_offset{};
	sf::Vector2f m_pawn_offset{};
	sf::Vector2f m_light_offset{};

	anim::AnimatedSprite m_core;
	anim::AnimatedSprite m_wires;
	anim::AnimatedSprite m_light;
	sf::Sprite m_scanline;
	sf::Sprite m_sprite;
	HealthDisplay m_health_display;
};

} // namespace fornani::gui
