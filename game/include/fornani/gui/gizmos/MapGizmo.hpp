
#pragma once

#include <fornani/gui/MiniMenu.hpp>
#include <fornani/systems/InputActionMap.hpp>
#include "fornani/gui/Gizmo.hpp"
#include "fornani/gui/MiniMap.hpp"
#include "fornani/gui/gizmos/MapInfoGizmo.hpp"
#include "fornani/particle/Chain.hpp"
#include "fornani/utils/BitFlags.hpp"
#include "fornani/utils/NineSlice.hpp"

namespace fornani::gui {

enum class MapGizmoFlags : std::uint8_t { pin_menu };
enum class MapGizmoControls : std::uint8_t { pan, menu, zoom_out, zoom_in, END };

class MapPlugin {
  public:
	MapPlugin(ResourceFinder& finder, std::string_view p, sf::IntRect lookup, audio::Pioneer sound);
	void update(audio::Soundboard& soundboard);
	void render(sf::RenderWindow& win, sf::Sprite& sprite, sf::Vector2f cam, sf::Vector2f origin, LightShader& shader, Palette& palette) const;

  private:
	Constituent constituent;
	util::RectPath m_path;
	util::Cooldown m_delay;
	audio::Pioneer m_sound;
};

struct MapControl {
	sf::Text text;
	MapGizmoControls control{};
	input::DigitalAction action{};
};

struct MiniMapLegend {
	MiniMapLegend(automa::ServiceProvider& svc);
	sf::Vector2f position{};
	std::vector<MapControl> instructions{};
	Animatable control_icon;
};

class MapGizmo : public Gizmo {
  public:
	MapGizmo(automa::ServiceProvider& svc, world::Map& map, player::Player& player);
	void update(automa::ServiceProvider& svc, [[maybe_unused]] player::Player& player, [[maybe_unused]] world::Map& map, sf::Vector2f position) override;
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, [[maybe_unused]] player::Player& player, LightShader& shader, Palette& palette, sf::Vector2f cam, bool foreground = false) override;
	bool handle_inputs(input::InputSystem& controller, [[maybe_unused]] audio::Soundboard& soundboard) override;

  private:
	void on_open(automa::ServiceProvider& svc, [[maybe_unused]] player::Player& player, [[maybe_unused]] world::Map& map) override;
	void on_close(automa::ServiceProvider& svc, [[maybe_unused]] player::Player& player, [[maybe_unused]] world::Map& map) override;
	void close_menu();
	std::vector<MapPlugin> m_plugins;
	std::unique_ptr<MapInfoGizmo> m_info{};
	std::optional<MiniMenu> m_menu{};
	MiniMap* m_minimap;
	std::vector<std::unique_ptr<vfx::Chain>> m_chains{};
	util::NineSlice m_map_screen;
	util::NineSlice m_map_shadow;
	util::RectPath m_path;
	util::RectPath m_motherboard_path;
	std::vector<sf::Vector2f> m_chain_offsets;
	sf::Sprite m_sprite;
	sf::Sprite m_plugin_sprite;
	sf::Sprite m_icon_sprite;
	struct {
		bool toggled{};
		util::BitFlags<MapGizmoFlags> general{};
		util::BitFlags<MapIconFlags> icon{};
	} m_flags{};
	struct {
		struct {
			Constituent top_left;
			Constituent top_right;
			Constituent bottom_left;
			Constituent bottom_right;
			Constituent motherboard;
		} gizmo;
	} m_constituents;
	struct {
		sf::Vector2i plugin;
		sf::Vector2i icon;
	} m_lookups;

	MiniMapLegend m_legend;

	automa::ServiceProvider* m_services;
};

} // namespace fornani::gui
