
#pragma once

#include "fornani/gui/Gizmo.hpp"
#include "fornani/utils/RectPath.hpp"

namespace fornani::gui {

class MapInfoGizmo : public Gizmo {
  public:
	MapInfoGizmo(automa::ServiceProvider& svc, world::Map& map, sf::Vector2f placement);
	void update(automa::ServiceProvider& svc, [[maybe_unused]] player::Player& player, [[maybe_unused]] world::Map& map, sf::Vector2f position) override;
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam, bool foreground = false) override;
	bool handle_inputs(config::ControllerMap& controller) override;

  private:
	util::RectPath m_clip_path;
	struct {
		sf::Sprite panel;
		sf::Sprite clip;
	} m_sprites;
	struct {
		Constituent left_clip;
		Constituent right_clip;
	} m_constituents;
	struct {
		sf::Text biome;
		sf::Text room;
	} m_text;
};

} // namespace fornani::gui
