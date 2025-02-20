
#pragma once

#include "fornani/gui/Gizmo.hpp"
#include "fornani/gui/MiniMap.hpp"
#include "fornani/utils/NineSlice.hpp"

namespace fornani::world {
class Map;
}

namespace fornani::gui {

class MapGizmo : public Gizmo {
  public:
	MapGizmo(automa::ServiceProvider& svc, world::Map& map);
	void update(automa::ServiceProvider& svc, [[maybe_unused]] player::Player& player, [[maybe_unused]] world::Map& map) override;
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) override;
	bool handle_inputs(config::ControllerMap& controller) override;

  private:
	MiniMap m_minimap;
	util::NineSlice m_map_screen;
	sf::Sprite m_sprite;
	struct {
		struct {
			sf::IntRect top_left;
			sf::IntRect top_right;
			sf::IntRect bottom_left;
			sf::IntRect bottom_right;
		} gizmo;
		struct {
			sf::IntRect left;
			sf::IntRect right;
			sf::IntRect top;
			sf::IntRect bottom;
		} chain;
		struct {
			sf::IntRect info;
			sf::IntRect left_grip;
			sf::IntRect right_grip;
			sf::IntRect motherboard;
		} extras;
		struct {
			sf::IntRect player;
			sf::IntRect chest;
			sf::IntRect save;
			sf::IntRect gunsmith;
			sf::IntRect bed;
			sf::IntRect gobe;
			sf::IntRect vendor;
			sf::IntRect door;
			sf::IntRect boss;
		} plugins;
	} m_rects;
};

} // namespace fornani::gui
