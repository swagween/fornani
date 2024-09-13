#include "NineSlice.hpp"
#include "../service/ServiceProvider.hpp"

namespace util {

void NineSlice::slice(automa::ServiceProvider& svc, int corner_factor, int edge_factor) {
	sprites.at(0).setTextureRect(sf::IntRect{{0, 0}, {corner_factor, corner_factor}});
	sprites.at(1).setTextureRect(sf::IntRect{{corner_factor, 0}, {edge_factor, corner_factor}});
	sprites.at(2).setTextureRect(sf::IntRect{{corner_factor + edge_factor, 0}, {corner_factor, corner_factor}});
	sprites.at(3).setTextureRect(sf::IntRect{{0, corner_factor}, {corner_factor, edge_factor}});
	sprites.at(4).setTextureRect(sf::IntRect{{corner_factor, corner_factor}, {edge_factor, edge_factor}});
	sprites.at(5).setTextureRect(sf::IntRect{{corner_factor + edge_factor, corner_factor}, {corner_factor, edge_factor}});
	sprites.at(6).setTextureRect(sf::IntRect{{0, corner_factor + edge_factor}, {corner_factor, corner_factor}});
	sprites.at(7).setTextureRect(sf::IntRect{{corner_factor, corner_factor + edge_factor}, {edge_factor, corner_factor}});
	sprites.at(8).setTextureRect(sf::IntRect{{corner_factor + edge_factor, corner_factor + edge_factor}, {corner_factor, corner_factor}});

	float fric{0.93f};
	gravitator = vfx::Gravitator(svc.constants.f_center_screen, sf::Color::Transparent, 0.4f);
	gravitator.collider.physics = components::PhysicsComponent(sf::Vector2<float>{fric, fric}, 2.0f);
}

void NineSlice::set_texture(sf::Texture& tex) {
	for (auto& sprite : sprites) { sprite.setTexture(tex); }
}

void NineSlice::set_origin(sf::Vector2<float> origin) {
	for (auto& sprite : sprites) { sprite.setOrigin(origin); }
}

void NineSlice::update(automa::ServiceProvider& svc, sf::Vector2<float> position, sf::Vector2<float> dimensions, float corner_dim, float edge_dim) {
	native_dimensions = dimensions;
	gravitator.set_target_position(position);
	gravitator.update(svc); 
	global_scale = 1.f - static_cast<float>(appear.get_cooldown()) / static_cast<float>(appearance_time);
	appear.update();
	auto middle = sf::Vector2<float>{(dimensions.x - 2.f * corner_dim) / edge_dim, (dimensions.y - 2.f * corner_dim) / edge_dim};
	auto half_edge = sf::Vector2<float>{edge_dim * 0.5f, edge_dim * 0.5f};

	// set origins
	sprites.at(0).setOrigin({corner_dim + middle.x, corner_dim + middle.y});
	sprites.at(1).setOrigin({half_edge.x, corner_dim + middle.y});
	sprites.at(2).setOrigin({-middle.x, corner_dim + middle.y});
	sprites.at(3).setOrigin({corner_dim + middle.x, half_edge.y});
	sprites.at(4).setOrigin(half_edge);
	sprites.at(5).setOrigin({-middle.x, half_edge.y});
	sprites.at(6).setOrigin({corner_dim + middle.x, -middle.y});
	sprites.at(7).setOrigin({half_edge.x, -middle.y});
	sprites.at(8).setOrigin({-middle.x, -middle.y});

	// set sizes for stretched 9-slice sprites
	sprites.at(0).setScale(sf::Vector2<float>{global_scale, global_scale});
	sprites.at(1).setScale(sf::Vector2<float>{middle.x, 1.f} * global_scale);
	sprites.at(2).setScale(sf::Vector2<float>{global_scale, global_scale});
	sprites.at(3).setScale(sf::Vector2<float>{1.f, middle.y} * global_scale);
	sprites.at(4).setScale(sf::Vector2<float>{middle.x, middle.y} * global_scale);
	sprites.at(5).setScale(sf::Vector2<float>{1.f, middle.y} * global_scale);
	sprites.at(6).setScale(sf::Vector2<float>{global_scale, global_scale});
	sprites.at(7).setScale(sf::Vector2<float>{middle.x, 1.f} * global_scale);
	sprites.at(8).setScale(sf::Vector2<float>{global_scale, global_scale});

	// set position for the 9-slice console box
	for (auto& sprite : sprites) { sprite.setPosition(gravitator.collider.physics.position); }
}

void NineSlice::render(sf::RenderWindow& win) const {
	for (auto& sprite : sprites) { win.draw(sprite); }
}

void NineSlice::start(automa::ServiceProvider& svc, sf::Vector2<float> position) {
	global_scale = 0.f;
	appear.start();
	random_offset = svc.random.random_vector_float(-32.f, 32.f);
	gravitator.set_position(position + random_offset);
	gravitator.collider.physics.velocity = svc.random.random_vector_float(-10.f, 10.f);
}

void NineSlice::end() {
	appear.start();
	set_scale(0.f);
}

void NineSlice::speed_up_appearance(int rate) {
	for (auto i{0}; i < rate; ++i) { appear.update(); }
}

} // namespace util
