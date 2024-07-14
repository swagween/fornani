#include "ItemWidget.hpp"
#include "../service/ServiceProvider.hpp"

namespace gui {

ItemWidget::ItemWidget(automa::ServiceProvider& svc) {
	dimensions = sf::Vector2<float>{32.f, 32.f};
	gun_dimensions = sf::Vector2<float>{48.f, 48.f};
	pad.x = svc.constants.screen_dimensions.x * 0.5f;
	pad.y = 230.f;
	end_position = sf::Vector2<float>{pad.x, svc.constants.screen_dimensions.y - pad.y - dimensions.y};
	bring_in();
	float fric{0.9f};
	gravitator = vfx::Gravitator(start_position, sf::Color::Transparent, 1.f);
	gravitator.collider.physics = components::PhysicsComponent(sf::Vector2<float>{fric, fric}, 2.0f);
	sprites.item.setTexture(svc.assets.t_items);
	sprites.gun.setTexture(svc.assets.t_guns);
	sprites.item.setOrigin(dimensions.x * 0.5f, dimensions.y * 0.5f);
	sprites.gun.setOrigin(gun_dimensions.x * 0.5f, gun_dimensions.y * 0.5f);
	sprites.item.setTextureRect(sf::IntRect({(id - 1) * (int)dimensions.x, 0}, {(int)dimensions.x, (int)dimensions.y}));
	gravitator.set_target_position(position);
	sparkler = vfx::Sparkler(svc, dimensions, svc.styles.colors.ui_white, "item");
	sticker.setFillColor(svc.styles.colors.console_blue);
	sticker.setRadius(32.f);
	sticker.setOrigin(sticker.getRadius(), sticker.getRadius());
	sticker.setPointCount(32);
}

void ItemWidget::update(automa::ServiceProvider& svc) {
	gravitator.set_target_position(position);
	gravitator.update(svc);
	sparkler.update(svc);
	sprites.item.setPosition(gravitator.collider.physics.position);
	sprites.gun.setPosition(gravitator.collider.physics.position);
	flags.test(WidgetFlags::gun) ? sticker.setPosition(sprites.gun.getPosition()) : sticker.setPosition(sprites.item.getPosition());
	sparkler.set_position(sticker.getPosition() - sparkler.get_dimensions() * 0.5f);
}

void ItemWidget::render(automa::ServiceProvider& svc, sf::RenderWindow& win) {
	win.draw(sticker);
	sprites.item.setTextureRect(sf::IntRect({(id - 1) * (int)dimensions.x, 0}, {(int)dimensions.x, (int)dimensions.y}));
	sprites.gun.setTextureRect(sf::IntRect({id * (int)gun_dimensions.x, 0}, {(int)gun_dimensions.x, (int)gun_dimensions.y}));
	if (flags.test(WidgetFlags::gun)) {
		win.draw(sprites.gun);
	} else {
		win.draw(sprites.item);
	}
	sparkler.render(svc, win, {});
}

void ItemWidget::reset(automa::ServiceProvider& svc) {
	start_position = {position.x, -64.f};
	set_position(start_position);
}

void ItemWidget::set_position(sf::Vector2<float> pos) {
	sprites.item.setPosition(pos);
	gravitator.set_position(pos);
}

void ItemWidget::bring_in() { position = end_position; }

void ItemWidget::send_out() { position = start_position; }

void ItemWidget::set_id(int new_id, bool is_gun) {
	id = new_id;
	if (is_gun) {
		flags.set(WidgetFlags::gun);
	} else {
		flags.reset(WidgetFlags::gun);
	}
	sprites.item.setTextureRect(sf::IntRect({(id - 1) * (int)dimensions.x, 0}, {(int)dimensions.x, (int)dimensions.y}));
	sprites.gun.setTextureRect(sf::IntRect({id * (int)gun_dimensions.x, 0}, {(int)gun_dimensions.x, (int)gun_dimensions.y}));
}

} // namespace gui
