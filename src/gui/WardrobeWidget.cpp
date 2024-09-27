#include "Portrait.hpp"
#include "../service/ServiceProvider.hpp"
#include "WardrobeWidget.hpp"
#include "../entities/player/Player.hpp"

namespace gui {

gui::WardrobeWidget::WardrobeWidget(automa::ServiceProvider& svc) {
	sprites.base.setTexture(svc.assets.t_wardrobe_base);
	sprites.hairstyle.setTexture(svc.assets.t_wardrobe_default_hair);
	sprites.pants.setTexture(svc.assets.t_wardrobe_green_pants);
	sprites.shirt.setTexture(svc.assets.t_wardrobe_blue_shirt);
	sprites.base.setOrigin(dimensions * 0.5f);
	sprites.hairstyle.setOrigin(dimensions * 0.5f);
	sprites.pants.setOrigin(dimensions * 0.5f);
	sprites.shirt.setOrigin(dimensions * 0.5f);
	background.setFillColor(svc.styles.colors.ui_black);
	background.setOutlineColor(svc.styles.colors.ui_white);
	background.setOutlineThickness(2);
	background.setSize(dimensions);
	background.setOrigin(dimensions * 0.5f);
}

void WardrobeWidget::update(automa::ServiceProvider& svc, player::Player& player) {
	switch (player.catalog.categories.wardrobe.get_variant(player::ApparelType::pants)) {
	case 0: sprites.pants.setTexture(svc.assets.t_wardrobe_green_pants); break;
	case 1: sprites.pants.setTexture(svc.assets.t_wardrobe_red_jeans); break;
	}
	switch (player.catalog.categories.wardrobe.get_variant(player::ApparelType::hairstyle)) {
	case 0: sprites.hairstyle.setTexture(svc.assets.t_wardrobe_default_hair); break;
	}
	switch (player.catalog.categories.wardrobe.get_variant(player::ApparelType::shirt)) {
	case 0: sprites.shirt.setTexture(svc.assets.t_wardrobe_blue_shirt); break;
	}
	player.catalog.categories.wardrobe.update(player.texture_updater);

	//set out_texture for updating console portrait
	nani.create(128, 256);
	nani.clear(sf::Color::Transparent);
	sprites.base.setOrigin({});
	sprites.shirt.setOrigin({});
	sprites.pants.setOrigin({});
	sprites.hairstyle.setOrigin({});
	nani.draw(sprites.base);
	nani.draw(sprites.pants);
	nani.draw(sprites.shirt);
	nani.draw(sprites.hairstyle);
	nani.display();
	out_nani = sf::Sprite(nani.getTexture());
	sprites.base.setOrigin(dimensions * 0.5f);
	sprites.hairstyle.setOrigin(dimensions * 0.5f);
	sprites.pants.setOrigin(dimensions * 0.5f);
	sprites.shirt.setOrigin(dimensions * 0.5f);
}

void WardrobeWidget::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam) {
	background.setPosition(position);
	sprites.base.setPosition(position);
	sprites.pants.setPosition(position);
	sprites.shirt.setPosition(position);
	sprites.hairstyle.setPosition(position);
	win.draw(background);
	win.draw(sprites.base);
	win.draw(sprites.pants);
	win.draw(sprites.shirt);
	win.draw(sprites.hairstyle);
}

} // namespace gui
