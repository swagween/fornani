
#pragma once

#include <fornani/graphics/Animatable.hpp>
#include <fornani/gui/InventorySelector.hpp>
#include <fornani/gui/MiniMenu.hpp>
#include <fornani/gui/NumberDisplay.hpp>
#include <fornani/gui/OrbDisplay.hpp>
#include <fornani/gui/ZoneCollection.hpp>
#include <fornani/gui/dialogs/IDialog.hpp>
#include <fornani/gui/dialogs/VendorConstituent.hpp>
#include <fornani/gui/gizmos/DescriptionGizmo.hpp>
#include <fornani/shader/FlatShader.hpp>
#include <fornani/shader/HoloShader.hpp>

namespace fornani::gui {

enum class BuilderConstituentType { docket, stage, inventory };
enum class BuilderZoneType { docket, stage, inventory, COUNT };

class BuilderDialog final : public IDialog {
  public:
	BuilderDialog(automa::ServiceProvider& svc, world::Map& map, player::Player& player, int vendor_id);
	void update(automa::ServiceProvider& svc, world::Map& map, player::Player& player, SceneContext& context) override;
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, player::Player& player, world::Map& map, LightShader& shader, Renderer& renderer) override;
	void refresh(automa::ServiceProvider& svc, player::Player& player, world::Map& map);

  private:
	void build_item(automa::ServiceProvider& svc, player::Player& player, dj::Json const& current_item);
	void switch_zones(int modulation);
	void debug();

  private:
	std::vector<std::string> m_docket{};
	std::vector<std::string> m_player_items{};
	std::string m_docket_item{};
	Animatable m_item_sprite;
	Animatable m_unknown;
	Animatable m_question_mark;
	std::vector<NumberDisplay> m_number_displays{};
	Animatable m_turntable;

	FlatShader m_flat_shader;
	HoloShader m_holo_shader;

	ZoneCollection<BuilderZoneType> m_zones;

	std::array<VendorConstituent, 3> m_constituents;

	std::optional<InventorySelector> m_selector{};
	std::optional<MiniMenu> m_item_menu{};
	std::unique_ptr<DescriptionGizmo> m_description;
};

} // namespace fornani::gui
