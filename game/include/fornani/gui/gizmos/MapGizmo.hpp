
#pragma once

#include "fornani/gui/Gizmo.hpp"
#include "fornani/gui/MiniMap.hpp"
#include "fornani/particle/Chain.hpp"
#include "fornani/utils/NineSlice.hpp"
#include "fornani/utils/RectPath.hpp"

namespace fornani::gui {

class MapGizmo : public Gizmo {
  public:
	MapGizmo(automa::ServiceProvider& svc, world::Map& map);
	void update(automa::ServiceProvider& svc, [[maybe_unused]] player::Player& player, [[maybe_unused]] world::Map& map, sf::Vector2f position) override;
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) override;
	bool handle_inputs(config::ControllerMap& controller) override;

  private:
	std::unique_ptr<MiniMap> m_minimap{};
	std::vector<std::unique_ptr<vfx::Chain>> m_chains{};
	util::NineSlice m_map_screen;
	util::NineSlice m_map_shadow;
	util::RectPath m_path;
	sf::Sprite m_sprite;
	struct {
		bool toggled{};
	} m_flags{};
	struct {
		struct {
			Constituent top_left;
			Constituent top_right;
			Constituent bottom_left;
			Constituent bottom_right;
		} gizmo;
		struct {
			Constituent left;
			Constituent right;
			Constituent top;
			Constituent bottom;
		} chain;
		struct {
			Constituent info;
			Constituent left_grip;
			Constituent right_grip;
			Constituent motherboard;
		} extras;
		struct {
			Constituent player;
			Constituent chest;
			Constituent save;
			Constituent gunsmith;
			Constituent bed;
			Constituent gobe;
			Constituent vendor;
			Constituent door;
			Constituent boss;
		} plugins;
	} m_constituents;
};

} // namespace fornani::gui
