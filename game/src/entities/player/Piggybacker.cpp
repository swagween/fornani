#include "fornani/entities/player/Piggybacker.hpp"
#include "fornani/entities/player/Player.hpp"
#include "fornani/service/ServiceProvider.hpp"

namespace fornani::player {

Piggybacker::Piggybacker(automa::ServiceProvider& svc, std::string_view label, sf::Vector2f position) : sprite{svc.assets.get_npc_texture(std::string{label})} {
	auto const& in_data = svc.data.npc[label];
	auto dimensions = sf::Vector2<int>{in_data["sprite_dimensions"][0].as<int>(), in_data["sprite_dimensions"][1].as<int>()};
	sprite.setTextureRect(sf::IntRect{{}, dimensions});
	gravitator = vfx::Gravitator(sf::Vector2f{}, sf::Color::Transparent, 0.8f);
	gravitator.collider.physics = components::PhysicsComponent(sf::Vector2f{0.9f, 0.9f}, 1.0f);
	gravitator.set_position(position);
	sprite.setOrigin(static_cast<sf::Vector2f>(dimensions) * 0.5f);
	offset.y = -static_cast<float>(dimensions.y) * 0.6f;
}

void Piggybacker::update(automa::ServiceProvider& svc, Player& player) {
	sprite.setScale({player.sprite.getScale().x * -1.f, player.sprite.getScale().y});
	offset.x = player.collider.dimensions.x * 0.5f;
	gravitator.set_target_position(player.collider.physics.position + offset);
	gravitator.update(svc);
}

void Piggybacker::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {
	sprite.setPosition(gravitator.position() - cam);
	win.draw(sprite);
}
} // namespace fornani::player
