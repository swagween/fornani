#include "fornani/gui/WardrobeWidget.hpp"
#include "fornani/entities/player/Player.hpp"
#include "fornani/gui/Portrait.hpp"
#include "fornani/service/ServiceProvider.hpp"

namespace fornani::gui {

gui::WardrobeWidget::WardrobeWidget(automa::ServiceProvider const& svc)
	: sprites{.base = sf::Sprite{svc.assets.get_texture("wardrobe_base")},
			  .shirt = sf::Sprite{svc.assets.get_texture("wardrobe_blue_shirt")},
			  .pants = sf::Sprite{svc.assets.get_texture("wardrobe_green_pants")},
			  .hairstyle = sf::Sprite{svc.assets.get_texture("wardrobe_default_hair")}},
	  out_nani{svc.assets.get_texture("null")} {
	background.setFillColor(svc.styles.colors.ui_black);
	background.setOutlineColor(svc.styles.colors.ui_white);
	background.setOutlineThickness(2.f);
	background.setSize(dimensions);
	background.setOrigin(dimensions * 0.5f);
	if (!nani.resize({128, 256})) { NANI_LOG_WARN(m_logger, "nani.resize() failed!"); }
}

void WardrobeWidget::update(automa::ServiceProvider& svc, player::Player& player) {
	switch (player.catalog.categories.wardrobe.get_variant(player::ApparelType::pants)) {
	case 0: sprites.pants.setTexture(svc.assets.get_texture("wardrobe_green_pants")); break;
	case 1: sprites.pants.setTexture(svc.assets.get_texture("wardrobe_red_jeans")); break;
	case 3: sprites.pants.setTexture(svc.assets.get_texture("wardrobe_chalcedony_skirt")); break;
	case 6: sprites.pants.setTexture(svc.assets.get_texture("wardrobe_punk_pants")); break;
	default: break; ;
	}
	switch (player.catalog.categories.wardrobe.get_variant(player::ApparelType::hairstyle)) {
	case 0: sprites.hairstyle.setTexture(svc.assets.get_texture("wardrobe_default_hair")); break;
	case 4: sprites.hairstyle.setTexture(svc.assets.get_texture("wardrobe_punk_hair")); break;
	case 7: sprites.hairstyle.setTexture(svc.assets.get_texture("wardrobe_ponytail")); break;
	default: break; ;
	}
	switch (player.catalog.categories.wardrobe.get_variant(player::ApparelType::shirt)) {
	case 0: sprites.shirt.setTexture(svc.assets.get_texture("wardrobe_blue_shirt")); break;
	case 2: sprites.shirt.setTexture(svc.assets.get_texture("wardrobe_chalcedony_tee")); break;
	case 5: sprites.shirt.setTexture(svc.assets.get_texture("wardrobe_punk_shirt")); break;
	default: break; ;
	}
	player.catalog.categories.wardrobe.update(player.texture_updater);

	// set out_texture for updating console portrait
	nani.clear(sf::Color::Transparent);
	nani.draw(sprites.base);
	nani.draw(sprites.pants);
	nani.draw(sprites.shirt);
	nani.draw(sprites.hairstyle);
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
