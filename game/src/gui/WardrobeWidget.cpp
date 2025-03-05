#include "fornani/gui/WardrobeWidget.hpp"
#include "fornani/entities/player/Player.hpp"
#include "fornani/gui/Portrait.hpp"
#include "fornani/service/ServiceProvider.hpp"

namespace fornani::gui {

gui::WardrobeWidget::WardrobeWidget(automa::ServiceProvider& svc)
	: m_sprites{.base{sf::Sprite{svc.assets.get_texture("wardrobe_base")}},
				.shirt{sf::Sprite{svc.assets.get_texture("wardrobe_blue_shirt")}},
				.pants{sf::Sprite{svc.assets.get_texture("wardrobe_green_pants")}},
				.hairstyle{sf::Sprite{svc.assets.get_texture("wardrobe_default_hair")}}},
	  out_nani{sf::Sprite{svc.assets.get_texture("null")}} {
	background.setFillColor(svc.styles.colors.ui_black);
	background.setOutlineColor(svc.styles.colors.ui_white);
	background.setOutlineThickness(2.f);
	background.setSize(dimensions);
	background.setOrigin(dimensions * 0.5f);
	if (!nani.resize({128, 256})) { NANI_LOG_WARN(m_logger, "nani.resize() failed!"); }
}

void WardrobeWidget::update(automa::ServiceProvider& svc, player::Player& player) {
	switch (player.catalog.wardrobe.get_variant(player::ApparelType::pants)) {
	case player::ClothingVariant::standard: m_sprites.pants.setTexture(svc.assets.get_texture("wardrobe_green_pants")); break;
	case player::ClothingVariant::red_jeans: m_sprites.pants.setTexture(svc.assets.get_texture("wardrobe_red_jeans")); break;
	case player::ClothingVariant::chalcedony_skirt: m_sprites.pants.setTexture(svc.assets.get_texture("wardrobe_chalcedony_skirt")); break;
	case player::ClothingVariant::punk_pants: m_sprites.pants.setTexture(svc.assets.get_texture("wardrobe_punk_pants")); break;
	default: break; ;
	}
	switch (player.catalog.wardrobe.get_variant(player::ApparelType::hairstyle)) {
	case player::ClothingVariant::standard: m_sprites.hairstyle.setTexture(svc.assets.get_texture("wardrobe_default_hair")); break;
	case player::ClothingVariant::punk_hair: m_sprites.hairstyle.setTexture(svc.assets.get_texture("wardrobe_punk_hair")); break;
	case player::ClothingVariant::ponytail: m_sprites.hairstyle.setTexture(svc.assets.get_texture("wardrobe_ponytail")); break;
	default: break; ;
	}
	switch (player.catalog.wardrobe.get_variant(player::ApparelType::shirt)) {
	case player::ClothingVariant::standard: m_sprites.shirt.setTexture(svc.assets.get_texture("wardrobe_blue_shirt")); break;
	case player::ClothingVariant::chalcedony_tee: m_sprites.shirt.setTexture(svc.assets.get_texture("wardrobe_chalcedony_tee")); break;
	case player::ClothingVariant::punk_shirt: m_sprites.shirt.setTexture(svc.assets.get_texture("wardrobe_punk_shirt")); break;
	default: break; ;
	}
	player.catalog.wardrobe.update(player.texture_updater);

	// set out_texture for updating console portrait
	nani.clear(sf::Color::Transparent);
	nani.draw(m_sprites.base);
	nani.draw(m_sprites.pants);
	nani.draw(m_sprites.shirt);
	nani.draw(m_sprites.hairstyle);
	nani.display();
	out_nani = sf::Sprite(nani.getTexture());
}

void WardrobeWidget::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam) {
	background.setPosition(position);
	win.draw(background);
	out_nani.setPosition(position);
	out_nani.setOrigin(dimensions * 0.5f);
	win.draw(out_nani);
}

} // namespace fornani::gui
