
#include <fornani/entities/player/Player.hpp>
#include <fornani/gui/hud/HUDItem.hpp>
#include <fornani/service/ServiceProvider.hpp>

namespace fornani::gui {

HUDItem::HUDItem(automa::ServiceProvider& svc, player::Player& player, std::string_view tag, sf::Vector2f root, sf::Vector2i dimensions) : HUDWidget{svc, player, tag, root, dimensions}, m_item_sprite{svc, "items_simple", {18, 18}} {
	m_item_sprite.center();
	if (p_animatable) { p_animatable->center(); }
}

void HUDItem::update(automa::ServiceProvider& svc, player::Player& player) { HUDWidget::update(svc, player); }

void HUDItem::render(automa::ServiceProvider& svc, player::Player& player, sf::RenderWindow& win, sf::Vector2f offset) {
	HUDWidget::render(svc, player, win, offset);
	if (!p_animatable) { return; }
	auto const pad = 2.f;
	for (int i = 0; i < player.catalog.inventory.get_quantity("equip_slot") + 1; ++i) {
		if (i < player.catalog.inventory.get_number_of_equipped_items()) { continue; }
		win.draw(*p_animatable);
		p_animatable->set_position(get_spot(i, offset));
	}
	for (auto [j, ei] : std::views::enumerate(player.catalog.inventory.equipped_items_view())) {
		auto item = player.catalog.inventory.find_item(ei);
		if (item == nullptr) { continue; }
		if (item->is_invisible()) { continue; }
		m_item_sprite.set_texture_rect(item->get_lookup());
		item->render(win, m_item_sprite.get_sprite(), get_spot(j, offset));
	}
}

sf::Vector2f HUDItem::get_spot(int i, sf::Vector2f offset) const { return sf::Vector2f{get_root().x - 2.f + 36.f * i + 18.f, -18.f} + offset; }

} // namespace fornani::gui
