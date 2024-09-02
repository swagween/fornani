#include "NineSlice.hpp"

namespace util {

void NineSlice::slice(int corner_factor, int edge_factor) {
	sprites.at(0).setTextureRect(sf::IntRect{{0, 0}, {corner_factor, corner_factor}});
	sprites.at(1).setTextureRect(sf::IntRect{{corner_factor, 0}, {edge_factor, corner_factor}});
	sprites.at(2).setTextureRect(sf::IntRect{{corner_factor + edge_factor, 0}, {corner_factor, corner_factor}});
	sprites.at(3).setTextureRect(sf::IntRect{{0, corner_factor}, {corner_factor, edge_factor}});
	sprites.at(4).setTextureRect(sf::IntRect{{corner_factor, corner_factor}, {edge_factor, edge_factor}});
	sprites.at(5).setTextureRect(sf::IntRect{{corner_factor + edge_factor, corner_factor}, {corner_factor, edge_factor}});
	sprites.at(6).setTextureRect(sf::IntRect{{0, corner_factor + edge_factor}, {corner_factor, corner_factor}});
	sprites.at(7).setTextureRect(sf::IntRect{{corner_factor, corner_factor + edge_factor}, {edge_factor, corner_factor}});
	sprites.at(8).setTextureRect(sf::IntRect{{corner_factor + edge_factor, corner_factor + edge_factor}, {corner_factor, corner_factor}});
}
void NineSlice::set_texture(sf::Texture& tex) {
	for (auto& sprite : sprites) { sprite.setTexture(tex); }
}

void NineSlice::set_origin(sf::Vector2<float> origin) {
	for (auto& sprite : sprites) { sprite.setOrigin(origin); }
}

void NineSlice::update(sf::Vector2<float> position, sf::Vector2<float> dimensions, int corner_dim, int edge_dim) {

	appear.update();
	global_scale = 1.f - static_cast<float>(appear.get_cooldown()) / 128.f;

	// set sizes for stretched 9-slice sprites
	sprites.at(0).setScale(sf::Vector2<float>{global_scale, global_scale});
	sprites.at(1).setScale(sf::Vector2<float>{(dimensions.x - 2.f * corner_dim) / edge_dim, 1.f} * global_scale);
	sprites.at(2).setScale(sf::Vector2<float>{global_scale, global_scale});
	sprites.at(3).setScale(sf::Vector2<float>{1.f, (dimensions.y - 2.f * corner_dim) / edge_dim} * global_scale);
	sprites.at(4).setScale(sf::Vector2<float>{(dimensions.x - 2.f * corner_dim) / edge_dim, (dimensions.y - 2.f * corner_dim) / edge_dim} * global_scale);
	sprites.at(5).setScale(sf::Vector2<float>{1.f, (dimensions.y - 2.f * corner_dim) / edge_dim} * global_scale);
	sprites.at(6).setScale(sf::Vector2<float>{global_scale, global_scale});
	sprites.at(7).setScale(sf::Vector2<float>{(dimensions.x - 2.f * corner_dim) / edge_dim, 1.f} * global_scale);
	sprites.at(8).setScale(sf::Vector2<float>{global_scale, global_scale});

	// set position for the 9-slice console box
	sprites.at(0).setPosition(sf::Vector2<float>{position.x, position.y} * global_scale);
	sprites.at(1).setPosition(sf::Vector2<float>{position.x + corner_dim, position.y} * global_scale);
	sprites.at(2).setPosition(sf::Vector2<float>{position.x + dimensions.x - corner_dim, position.y} * global_scale);

	sprites.at(3).setPosition(sf::Vector2<float>{position.x, position.y + corner_dim} * global_scale);
	sprites.at(4).setPosition(sf::Vector2<float>{position.x + corner_dim, position.y + corner_dim} * global_scale);
	sprites.at(5).setPosition(sf::Vector2<float>{position.x + dimensions.x - corner_dim, position.y + corner_dim} * global_scale);

	sprites.at(6).setPosition(sf::Vector2<float>{position.x, position.y + dimensions.y - corner_dim} * global_scale);
	sprites.at(7).setPosition(sf::Vector2<float>{position.x + corner_dim, position.y + dimensions.y - corner_dim} * global_scale);
	sprites.at(8).setPosition(sf::Vector2<float>{position.x + dimensions.x - corner_dim, position.y + dimensions.y - corner_dim} * global_scale);
}
void NineSlice::render(sf::RenderWindow& win) const {
	for (auto& sprite : sprites) { win.draw(sprite); }
}

} // namespace util
