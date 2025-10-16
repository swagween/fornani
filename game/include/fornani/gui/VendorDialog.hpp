
#pragma once

#include <SFML/Graphics.hpp>
#include <fornani/entities/animation/AnimatedSprite.hpp>
#include <fornani/entity/NPC.hpp>
#include <fornani/gui/InventorySelector.hpp>
#include <fornani/gui/MiniMenu.hpp>
#include <fornani/gui/OrbDisplay.hpp>
#include <fornani/gui/console/Console.hpp>
#include <fornani/gui/gizmos/DescriptionGizmo.hpp>
#include <fornani/gui/gizmos/InventoryGizmo.hpp>
#include <fornani/utils/RectPath.hpp>
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

enum class VendorDialogStatus : std::uint8_t { opened, made_sale };
enum class VendorState : std::uint8_t { buy, sell };
enum class VendorConstituentType : std::uint8_t { portrait, wares, description, name, core, selection, nani };

struct VendorConstituent : public Drawable {
	VendorConstituent(automa::ServiceProvider& svc, std::string_view label, sf::IntRect lookup, int speed = 128, util::InterpolationType type = util::InterpolationType::quadratic);
	util::RectPath path;
	void update();
	void render(sf::RenderWindow& win);
};

class VendorDialog {
  public:
	VendorDialog(automa::ServiceProvider& svc, world::Map& map, player::Player& player, int vendor_id);
	void update(automa::ServiceProvider& svc, world::Map& map, player::Player& player);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, player::Player& player, world::Map& map);
	void close(automa::ServiceProvider& svc);
	void update_table(player::Player& player, world::Map& map, bool new_dim);
	void refresh(player::Player& player, world::Map& map) const;

	[[nodiscard]] auto is_open() const -> bool { return flags.test(VendorDialogStatus::opened); }
	[[nodiscard]] auto is_buying() const -> bool { return m_state == VendorState::buy; }
	[[nodiscard]] auto is_selling() const -> bool { return m_state == VendorState::sell; }
	[[nodiscard]] auto made_sale() const -> bool { return flags.test(VendorDialogStatus::made_sale); }
	[[nodiscard]] auto made_profit() const -> bool { return balance > 0.f; }
	[[nodiscard]] auto is_opening() const -> bool { return m_intro.running(); }

  private:
	VendorState m_state{};
	InventorySelector m_buy_selector;
	InventorySelector m_sell_selector;
	std::optional<MiniMenu> m_item_menu{};
	util::Cooldown m_intro{};
	util::BitFlags<VendorDialogStatus> flags{};
	std::unique_ptr<DescriptionGizmo> m_description;
	Drawable m_artwork;
	Drawable m_selector_sprite;
	Drawable m_vendor_portrait;
	OrbDisplay m_orb_display;
	sf::RectangleShape m_background{};

	NPC* my_npc;

	int vendor_id{};
	int npc_id{};

	float sale_price{};
	float balance{};
	sf::Vector2f portrait_position{44.f, 18.f};
	sf::Vector2f bring_in{};

	std::array<VendorConstituent, 7> m_constituents;

	std::array<std::array<int, 8>, 4> m_items_list{};
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
		int items_per_row{12};
	} ui_constants{};
	struct {
		anim::AnimatedSprite sprite;
	} orb;

	io::Logger m_logger{"Vendor"};
};

} // namespace fornani::gui
