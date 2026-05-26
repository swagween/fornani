
#pragma once

#include <SFML/Graphics.hpp>
#include <fornani/entities/animation/AnimatedSprite.hpp>
#include <fornani/entity/NPC.hpp>
#include <fornani/gui/InventorySelector.hpp>
#include <fornani/gui/NumberDisplay.hpp>
#include <fornani/gui/OrbDisplay.hpp>
#include <fornani/gui/console/Console.hpp>
#include <fornani/gui/dialogs/IDialog.hpp>
#include <fornani/gui/dialogs/VendorConstituent.hpp>
#include <fornani/gui/gizmos/DescriptionGizmo.hpp>
#include <optional>

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani::player {
class Player;
}

namespace fornani::npc {
class Vendor;
}

namespace fornani::gui {

enum class VendorConstituentType { portrait, wares, description, name, core, selection, nani };

struct VendorItem {
	int id{};
	std::optional<NumberDisplay> price_display{};
};

class VendorDialog final : public IDialog {
  public:
	VendorDialog(automa::ServiceProvider& svc, world::Map& map, player::Player& player, int vendor_id);
	void update(automa::ServiceProvider& svc, world::Map& map, player::Player& player, SceneContext& context) override;
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, player::Player& player, world::Map& map, LightShader& shader) override;
	void refresh(automa::ServiceProvider& svc, player::Player& player, world::Map& map) override;

  private:
	InventorySelector m_buy_selector;
	InventorySelector m_sell_selector;
	std::optional<MiniMenu> m_item_menu{};
	std::unique_ptr<DescriptionGizmo> m_description;
	OrbDisplay m_orb_display;

	NPC* my_npc;

	int npc_id{};

	float sale_price{};
	sf::Vector2f portrait_position{44.f, 18.f};
	sf::Vector2f bring_in{};

	std::array<VendorConstituent, 7> m_constituents;

	std::array<VendorItem, 8> m_vendor_items_list{};
	std::array<int, 24> m_player_items_list{};
	Drawable m_item_sprite;

	struct {
		sf::Text vendor_name;
		sf::Text buy_tab;
		sf::Text sell_tab;
		sf::Text orb_count;
		sf::Text price;
		sf::Text price_number;
		sf::Text item_label;
	} text;

	struct {
		sf::Vector2f item_label_position{232.f, 320.f};
		sf::Vector2f price_position{232.f, 345.f};
		sf::Vector2f rarity_pad{32.f, 32.f};
		float buffer{40.f};
	} ui_constants{};

	struct {
		anim::AnimatedSprite sprite;
	} orb;

	io::Logger m_logger{"Vendor"};
};

} // namespace fornani::gui
