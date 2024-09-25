#include "Portrait.hpp"
#include "../service/ServiceProvider.hpp"
#include "WardrobeWidget.hpp"

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
}

void WardrobeWidget::update(automa::ServiceProvider& svc, player::Player& player) {}

void WardrobeWidget::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam) {
	sprites.base.setPosition(position);
	sprites.pants.setPosition(position);
	sprites.shirt.setPosition(position);
	sprites.hairstyle.setPosition(position);
	win.draw(sprites.base);
	win.draw(sprites.pants);
	win.draw(sprites.shirt);
	win.draw(sprites.hairstyle);
}

} // namespace gui
