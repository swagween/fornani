#pragma once
#include <SFML/Graphics.hpp>
#include "../particle/Gravitator.hpp"
#include "Portrait.hpp"
#include "Selector.hpp"
#include "Console.hpp"
#include "MiniMenu.hpp"
#include "../entities/animation/AnimatedSprite.hpp"
#include <memory>

namespace automa {
struct ServiceProvider;
}
namespace player {
class Player;
}
namespace npc {
class Vendor;
}

namespace gui {
enum class VendorDialogStatus { opened, made_sale };
enum class VendorState { sell, buy };
class VendorDialog {
  public:
	VendorDialog(automa::ServiceProvider& svc, world::Map& map, player::Player& player, int vendor_id);
	void update(automa::ServiceProvider& svc, world::Map& map, player::Player& player);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, player::Player& player);
	void close();
	void update_table(player::Player& player, bool new_dim);
	[[nodiscard]] auto is_open() const -> bool { return flags.test(VendorDialogStatus::opened); }
	[[nodiscard]] auto made_sale() const -> bool { return flags.test(VendorDialogStatus::made_sale); }
  private:
	Selector selector{};
	Console info;
	MiniMenu item_menu;
	util::BitFlags<VendorDialogStatus> flags{};
	sf::Sprite artwork{};
	sf::Sprite ui{};
	Portrait portrait;
	VendorState state{};
	int vendor_id{};
	int npc_id{};
	float sale_price{};
	std::unordered_map<int, int> get_npc_id{};
	sf::Vector2<float> portrait_position{44.f, 18.f};
	struct {
		sf::Text vendor_name{};
		sf::Text buy_tab{};
		sf::Text sell_tab{};
		sf::Text orb_count{};
		sf::Text price{};
		sf::Text price_number{};
		sf::Text value{};
		sf::Text item_label{};
	} text{};
	struct {
		sf::Vector2<float> item_label_position{232.f, 320.f};
		sf::Vector2<float> price_position{232.f, 345.f};
		sf::Vector2<float> rarity_pad{32.f, 32.f};
		float buffer{40.f};
		int items_per_row{12};
	} ui_constants{};
	struct {
		anim::AnimatedSprite sprite{};
	} orb{};
};

} // namespace gui