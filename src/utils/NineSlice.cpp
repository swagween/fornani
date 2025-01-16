#include "NineSlice.hpp"
#include "../service/ServiceProvider.hpp"

namespace util {

NineSlice::NineSlice(automa::ServiceProvider& svc, int corner_factor, int edge_factor) : sprite{svc.assets.t_blue_console}, corner_factor(corner_factor), edge_factor(edge_factor) {
	float fric{0.93f};
	gravitator = vfx::Gravitator(svc.constants.f_center_screen, sf::Color::Transparent, 0.4f);
	gravitator.collider.physics = components::PhysicsComponent(sf::Vector2<float>{fric, fric}, 2.0f);
}

void NineSlice::set_texture(sf::Texture& tex) { sprite.setTexture(tex); }

void NineSlice::set_origin(sf::Vector2<float> origin) { sprite.setOrigin(origin); }

void NineSlice::update(automa::ServiceProvider& svc, sf::Vector2<float> position, sf::Vector2<float> dimensions, float corner_dim, float edge_dim) {
	corner_dimensions = corner_dim;
	edge_dimensions = edge_dim;
	native_dimensions = dimensions;
	gravitator.set_target_position(position);
	gravitator.update(svc);
	if (native_scale < 1.f) { global_scale = 1.f - appear.get_cubic_normalized(); }
	appear.update();

	// set position for the 9-slice console box
	sprite.setPosition(gravitator.collider.physics.position);
}

void NineSlice::render(sf::RenderWindow& win) {
	
	auto middle = sf::Vector2<float>{(native_dimensions.x - 2.f * corner_dimensions) / edge_dimensions, (native_dimensions.y - 2.f * corner_dimensions) / edge_dimensions};
	auto half_edge = sf::Vector2<float>{edge_dimensions * 0.5f, edge_dimensions * 0.5f};

	// draw
	sprite.setTextureRect(sf::IntRect{{0, 0}, {corner_factor, corner_factor}});
	sprite.setOrigin({corner_dimensions + middle.x, corner_dimensions + middle.y});
	sprite.setScale(sf::Vector2<float>{global_scale, global_scale});
	win.draw(sprite);
	sprite.setTextureRect(sf::IntRect{{corner_factor, 0}, {edge_factor, corner_factor}});
	sprite.setOrigin({half_edge.x, corner_dimensions + middle.y});
	sprite.setScale(sf::Vector2<float>{middle.x, 1.f} * global_scale);
	win.draw(sprite);
	sprite.setTextureRect(sf::IntRect{{corner_factor + edge_factor, 0}, {corner_factor, corner_factor}});
	sprite.setOrigin({-middle.x, corner_dimensions + middle.y});
	sprite.setScale(sf::Vector2<float>{global_scale, global_scale});
	win.draw(sprite);
	sprite.setTextureRect(sf::IntRect{{0, corner_factor}, {corner_factor, edge_factor}});
	sprite.setOrigin({corner_dimensions + middle.x, half_edge.y});
	sprite.setScale(sf::Vector2<float>{1.f, middle.y} * global_scale);
	win.draw(sprite);
	sprite.setTextureRect(sf::IntRect{{corner_factor, corner_factor}, {edge_factor, edge_factor}});
	sprite.setOrigin(half_edge);
	sprite.setScale(sf::Vector2<float>{middle.x, middle.y} * global_scale);
	win.draw(sprite);
	sprite.setTextureRect(sf::IntRect{{corner_factor + edge_factor, corner_factor}, {corner_factor, edge_factor}});
	sprite.setOrigin({-middle.x, half_edge.y});
	sprite.setScale(sf::Vector2<float>{1.f, middle.y} * global_scale);
	win.draw(sprite);
	sprite.setTextureRect(sf::IntRect{{0, corner_factor + edge_factor}, {corner_factor, corner_factor}});
	sprite.setOrigin({corner_dimensions + middle.x, -middle.y});
	sprite.setScale(sf::Vector2<float>{global_scale, global_scale});
	win.draw(sprite);
	sprite.setTextureRect(sf::IntRect{{corner_factor, corner_factor + edge_factor}, {edge_factor, corner_factor}});
	sprite.setOrigin({half_edge.x, -middle.y});
	sprite.setScale(sf::Vector2<float>{middle.x, 1.f} * global_scale);
	win.draw(sprite);
	sprite.setTextureRect(sf::IntRect{{corner_factor + edge_factor, corner_factor + edge_factor}, {corner_factor, corner_factor}});
	sprite.setOrigin({-middle.x, -middle.y});
	sprite.setScale(sf::Vector2<float>{global_scale, global_scale});
	win.draw(sprite);
}

void NineSlice::start(automa::ServiceProvider& svc, sf::Vector2<float> position, float start_scale, sf::Vector2<int> direction, float border) {
	native_scale = start_scale;
	global_scale = start_scale;
	appear.start();
	auto x = direction.x == -1 ? -(native_dimensions.x + border) : direction.x == 1 ? svc.constants.f_screen_dimensions.x + native_dimensions.x + border : position.x;
	auto y = direction.y == -1 ? -(native_dimensions.y + border) : direction.y == 1 ? svc.constants.f_screen_dimensions.y + native_dimensions.y + border : position.y;
	gravitator.set_position({x, y});
}

void NineSlice::end() {
	appear.start();
	set_scale(native_scale);
}

void NineSlice::speed_up_appearance(int rate) {
	for (auto i{0}; i < rate; ++i) { appear.update(); }
}

} // namespace util
