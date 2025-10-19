
#pragma once

#include "fornani/entities/animation/AnimatedSprite.hpp"
#include "fornani/gui/Gizmo.hpp"
#include "fornani/gui/WardrobeWidget.hpp"
#include "fornani/gui/gizmos/OutfitterGizmo.hpp"

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
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, [[maybe_unused]] player::Player& player, LightShader& shader, Palette& palette, sf::Vector2f cam, bool foreground = false) override;
	bool handle_inputs(config::ControllerMap& controller, [[maybe_unused]] audio::Soundboard& soundboard) override;

  private:
	void on_open(automa::ServiceProvider& svc, [[maybe_unused]] player::Player& player, [[maybe_unused]] world::Map& map) override;
	void on_close(automa::ServiceProvider& svc, [[maybe_unused]] player::Player& player, [[maybe_unused]] world::Map& map) override;
	std::unique_ptr<OutfitterGizmo> m_outfitter{};
	util::RectPath m_path;
	bool m_wardrobe_update{};
	sf::Vector2f m_nani_offset{};
	sf::Vector2f m_pawn_offset{};
	sf::Vector2f m_light_offset{};

	anim::AnimatedSprite m_core;
	anim::AnimatedSprite m_light;
	sf::Sprite m_scanline;
	sf::Sprite m_sprite;
	sf::Sprite m_apparel_sprite;
	HealthDisplay m_health_display;
};

} // namespace fornani::gui
