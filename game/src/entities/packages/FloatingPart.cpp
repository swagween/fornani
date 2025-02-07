#include "fornani/entities/packages/FloatingPart.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/entities/player/Player.hpp"
#include "fornani/level/Map.hpp"
#include <algorithm>
#include <numbers>

namespace entity {

FloatingPart::FloatingPart(sf::Texture& tex, float force, float friction, sf::Vector2<float> offset) : sprite(tex) {
	sprite.setTexture(tex);
	sprite.setOrigin(sprite.getLocalBounds().getCenter());
	gravitator = std::make_unique<vfx::Gravitator>(sf::Vector2<float>{}, sf::Color::Yellow, force);
	gravitator->collider.physics = components::PhysicsComponent(sf::Vector2<float>{friction, friction}, 1.0f);
	gravitator->collider.physics.maximum_velocity = sf::Vector2<float>(20.f, 20.f);
	left = offset;
	right = offset;
	right.x *= -1.f;
	init = true;
	debugbox.setFillColor(sf::Color::Transparent);
	debugbox.setOutlineColor(sf::Color::Red);
	debugbox.setOutlineThickness(-1);
}

void FloatingPart::update(automa::ServiceProvider& svc, world::Map& map, player::Player& player, dir::Direction direction, sf::Vector2<float> scale, sf::Vector2<float> position) {
	if (init) {
		gravitator->set_position(position + actual);
		movement.time = svc.random.random_range_float(0.f, 2.f * static_cast<float>(std::numbers::pi));
		init = false;
	}
	actual = direction.lr == dir::LR::left ? position + left : position + right;
	movement.time += movement.rate;
	auto tweak = movement.magnitude * std::sin(movement.time);
	if (tweak == 0.f) { movement.time = 0.f; }
	actual.y += tweak;
	gravitator->set_target_position(actual);
	gravitator->update(svc);
	sprite.setScale(scale);
	if(hitbox) {
		if (player.collider.hurtbox.overlaps(hitbox.value())) { player.hurt(); }
	}
	if (shieldbox) {
		for (auto& proj : map.active_projectiles) {
			if (proj.get_bounding_box().overlaps(shieldbox.value())) {
				if (!proj.destruction_initiated()) {
					map.effects.push_back(entity::Effect(svc, proj.get_destruction_point() + proj.get_position(), {}, 0, 6));
					if (proj.get_direction().up_or_down()) { map.effects.back().rotate(); }
					svc.soundboard.flags.world.set(audio::World::hard_hit);
				}
				proj.destroy(false);
			}
		}
	}
}

void FloatingPart::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam) {
	sprite.setPosition(gravitator->position() - cam);
	win.draw(sprite);
	if(svc.greyblock_mode()) {
		gravitator->render(svc, win, cam);
		if (hitbox) {
			debugbox.setSize(hitbox.value().get_dimensions());
			debugbox.setPosition(hitbox.value().get_position());
			win.draw(debugbox);
		}
		if (shieldbox) {
			debugbox.setSize(shieldbox.value().get_dimensions());
			debugbox.setPosition(shieldbox.value().get_position());
			win.draw(debugbox);
		}
	}
}

void FloatingPart::set_shield(sf::Vector2<float> dim, sf::Vector2<float> pos) {
	if (dim.x == 0.f || dim.y == 0.f) { dim = sprite.getLocalBounds().size; }
	if (!shieldbox) { shieldbox = shape::Shape(dim); }
	if (pos.x == 0.f && pos.y == 0.f) { pos = gravitator->position() - sprite.getLocalBounds().getCenter(); }
	shieldbox.value().set_position(pos);
}

void FloatingPart::set_hitbox(sf::Vector2<float> dim, sf::Vector2<float> pos) {
	if (dim.x == 0.f || dim.y == 0.f) { dim = sprite.getLocalBounds().size; }
	if (!hitbox) { hitbox = shape::Shape(dim); }
	if (pos.x == 0.f && pos.y == 0.f) { pos = gravitator->position() - sprite.getLocalBounds().getCenter(); }
	hitbox.value().set_position(pos);
}

void FloatingPart::move(sf::Vector2<float> distance) { gravitator->set_target_position(gravitator->position() + distance); }

} // namespace entity
