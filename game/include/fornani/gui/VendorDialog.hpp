
#pragma once

#include "fornani/entities/animation/AnimatedSprite.hpp"
#include "fornani/gui/Console.hpp"
#include "fornani/gui/MiniMenu.hpp"
#include "fornani/gui/Portrait.hpp"
#include "fornani/gui/Selector.hpp"
#include "fornani/particle/Gravitator.hpp"

#include <SFML/Graphics.hpp>

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
namespace fornani::flfx {
class Transition;
}

namespace fornani::gui {
enum class VendorDialogStatus : uint8_t { opened, made_sale };
enum class VendorState : uint8_t { sell, buy };
class VendorDialog {
  public:
	VendorDialog(automa::ServiceProvider& svc, world::Map& map, player::Player& player, int vendor_id);
	void update(automa::ServiceProvider& svc, world::Map& map, player::Player& player);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, player::Player& player, world::Map& map);
	void close();
	void update_table(player::Player& player, world::Map& map, bool new_dim);
	void refresh(player::Player& player, world::Map& map) const;
	[[nodiscard]] auto is_open() const -> bool { return flags.test(VendorDialogStatus::opened); }
	[[nodiscard]] auto made_sale() const -> bool { return flags.test(VendorDialogStatus::made_sale); }
	[[nodiscard]] auto made_profit() const -> bool { return balance > 0.f; }
	[[nodiscard]] auto opening() const -> bool { return intro.running() || bring_in_cooldown.running(); }

  private:
	struct {
		Selector buy;
		Selector sell;
	} selectors;
	Console info;
	std::optional<MiniMenu> m_item_menu{};
	util::Cooldown intro{200};
	util::Cooldown bring_in_cooldown{200};
	util::BitFlags<VendorDialogStatus> flags{};
	sf::Sprite artwork;
	sf::Sprite ui;
	Portrait portrait;
	VendorState state{};
	int vendor_id{};
	int npc_id{};
	bool init{};
	float sale_price{};
	float balance{};
	std::unordered_map<int, int> get_npc_id{};
	sf::Vector2<float> portrait_position{44.f, 18.f};
	sf::Vector2<float> bring_in{};
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
		sf::Vector2<float> item_label_position{232.f, 320.f};
		sf::Vector2<float> price_position{232.f, 345.f};
		sf::Vector2<float> rarity_pad{32.f, 32.f};
		float buffer{40.f};
		int items_per_row{12};
	} ui_constants{};
	struct {
		anim::AnimatedSprite sprite;
	} orb;
};

} // namespace fornani::gui
