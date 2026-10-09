
#pragma once

#include <fornani/graphics/Animatable.hpp>
#include <fornani/gui/InventorySelector.hpp>
#include <fornani/gui/MiniMenu.hpp>
#include <fornani/gui/dialogs/IDialog.hpp>
#include <fornani/gui/dialogs/VendorConstituent.hpp>
#include <fornani/shader/FlatShader.hpp>

namespace fornani::gui {

enum class LandingPointFlags : std::uint8_t { selected, locked };

struct LandingPoint {
	sf::Text tag;
	sf::Vector2f position{};
	int destination{};
	util::BitFlags<LandingPointFlags> flags{};
};

class AirTravelDialog final : public IDialog {
  public:
	AirTravelDialog(automa::ServiceProvider& svc, world::Map& map, player::Player& player, int vendor_id);
	void update(automa::ServiceProvider& svc, world::Map& map, player::Player& player, SceneContext& context) override;
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, player::Player& player, world::Map& map, LightShader& shader, Renderer& renderer) override;

  private:
	void debug();

  private:
	Drawable m_backdrop;
	Animatable m_marker;
	Animatable m_indicator;
	std::vector<LandingPoint> m_destinations{};
	std::optional<int> m_target_room{};
	util::Cooldown m_made_selection;

	FlatShader m_flat_shader;

	std::optional<InventorySelector> m_selector{};
};

} // namespace fornani::gui
