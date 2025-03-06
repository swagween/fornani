
#pragma once

#include "fornani/entities/animation/AnimatedSprite.hpp"
#include "fornani/entities/player/Wardrobe.hpp"
#include "fornani/gui/Gizmo.hpp"

namespace fornani::gui {

class OutfitterGizmo : public Gizmo {
  public:
	OutfitterGizmo(automa::ServiceProvider& svc, world::Map& map, sf::Vector2f placement);
	void update(automa::ServiceProvider& svc, [[maybe_unused]] player::Player& player, [[maybe_unused]] world::Map& map, sf::Vector2f position) override;
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, [[maybe_unused]] player::Player& player, sf::Vector2f cam, bool foreground = false) override;
	bool handle_inputs(config::ControllerMap& controller, [[maybe_unused]] audio::Soundboard& soundboard) override;

	void close();

  private:
	sf::Sprite m_sprite;
	std::array<Constituent, static_cast<int>(player::ApparelType::END)> m_sliders;
	Constituent m_selector;
	Constituent m_row;
	util::RectPath m_path;
	anim::AnimatedSprite m_wires;
	int m_max_slots{};
};

} // namespace fornani::gui
