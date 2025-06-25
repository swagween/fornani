
#include "fornani/entities/packages/FloatingPart.hpp"
#include "fornani/entities/player/Player.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/utils/Random.hpp"
#include "fornani/world/Map.hpp"

#include <numbers>

#include "fornani/utils/Random.hpp"

namespace fornani::entity {

FloatingPart::FloatingPart(sf::Texture const& tex, float force, float friction, sf::Vector2<float> offset, int id) : sprite{tex}, textured{true}, init{true}, m_id{id} {
	sprite->setOrigin(sprite->getLocalBounds().getCenter());
	gravitator = std::make_unique<vfx::Gravitator>(sf::Vector2<float>{}, sf::Color::Yellow, force);
	gravitator->collider.physics = components::PhysicsComponent(sf::Vector2<float>{friction, friction}, 1.0f);
	gravitator->collider.physics.maximum_velocity = sf::Vector2<float>(20.f, 20.f);
	left = offset;
	right = offset;
	right.x *= -1.f;
	debugbox.setFillColor(sf::Color::Transparent);
	debugbox.setOutlineColor(sf::Color::Red);
	debugbox.setOutlineThickness(-1);
}

FloatingPart::FloatingPart(sf::Texture const& tex, sf::Vector2i dimensions, std::vector<anim::Parameters> params, std::vector<std::string_view> labels, float force, float friction, sf::Vector2<float> offset, int id)
	: textured{true}, init{true}, m_id{id} {
	animated_sprite = anim::AnimatedSprite(tex, dimensions);
	gravitator = std::make_unique<vfx::Gravitator>(sf::Vector2<float>{}, sf::Color::Yellow, force);
	gravitator->collider.physics = components::PhysicsComponent(sf::Vector2<float>{friction, friction}, 1.0f);
	gravitator->collider.physics.maximum_velocity = sf::Vector2<float>(20.f, 20.f);
	left = offset;
	right = offset;
	right.x *= -1.f;
	if (labels.size() < params.size()) { return; }
	if (!animated_sprite) { return; }
	auto ctr{0};
	for (auto& param : params) {
		animated_sprite->push_params(labels.at(ctr), param);
		++ctr;
	}
}

FloatingPart::FloatingPart(sf::Color color, sf::Vector2f dimensions, float force, float friction, sf::Vector2<float> offset, int id) : textured{false}, init{true}, m_id{id} {
	drawbox = sf::RectangleShape();
	drawbox->setSize(dimensions);
	drawbox->setFillColor(color);
	gravitator = std::make_unique<vfx::Gravitator>(sf::Vector2<float>{}, sf::Color::Yellow, force);
	gravitator->collider.physics = components::PhysicsComponent(sf::Vector2<float>{friction, friction}, 1.0f);
	gravitator->collider.physics.maximum_velocity = sf::Vector2<float>(20.f, 20.f);
	left = offset;
	right = offset;
	right.x *= -1.f;
}

void FloatingPart::update(automa::ServiceProvider& svc, world::Map& map, player::Player& player, Direction direction, sf::Vector2<float> scale, sf::Vector2<float> position) {
	if (init) {
		gravitator->set_position(position + actual);
		movement.time = util::random::random_range_float(0.f, 2.f * static_cast<float>(std::numbers::pi));
		init = false;
	}
	actual = direction.lnr == LNR::left ? position + left : position + right;
	movement.time += movement.rate;
	auto const tweak = movement.magnitude * std::sin(movement.time);
	if (tweak == 0.f) { movement.time = 0.f; }
	actual.y += tweak;
	gravitator->set_target_position(actual);
	gravitator->update(svc);
	if (animated_sprite) {
		animated_sprite->update(gravitator->position());
		animated_sprite->set_scale(scale);
	}
	if (sprite) { sprite->setScale(scale); }
	if (hitbox) {
		if (player.collider.hurtbox.overlaps(hitbox.value())) { player.hurt(); }
	}
	if (shieldbox) {
		for (auto& proj : map.active_projectiles) {
			if (proj.get_bounding_box().overlaps(shieldbox.value())) {
				if (!proj.destruction_initiated()) {
					map.effects.push_back(entity::Effect(svc, "inv_hit", proj.get_destruction_point() + proj.get_position(), {}, 0, 6));
					if (proj.get_direction().up_or_down()) { map.effects.back().rotate(); }
					svc.soundboard.flags.world.set(audio::World::hard_hit);
				}
				proj.destroy(false);
			}
		}
	}
}

void FloatingPart::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam) {
	if (sprite) { sprite->setPosition(gravitator->position() - cam); }
	if (sprite) { win.draw(*sprite); }
	if (animated_sprite) { animated_sprite->render(svc, win, cam); }
	if (drawbox) { drawbox->setPosition(gravitator->position() - cam); }
	if (drawbox) { win.draw(*drawbox); }
	if (svc.greyblock_mode()) {
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
	if ((dim.x == 0.f || dim.y == 0.f) && sprite) { dim = sprite->getLocalBounds().size; }
	if (!shieldbox) { shieldbox = shape::Shape(dim); }
	if (pos.x == 0.f && pos.y == 0.f && sprite) { pos = gravitator->position() - sprite->getLocalBounds().getCenter(); }
	shieldbox.value().set_position(pos);
}

void FloatingPart::set_hitbox(sf::Vector2<float> dim, sf::Vector2<float> pos) {
	if ((dim.x == 0.f || dim.y == 0.f) && sprite) { dim = sprite->getLocalBounds().size; }
	if (!hitbox) { hitbox = shape::Shape(dim); }
	if (pos.x == 0.f && pos.y == 0.f && sprite) { pos = gravitator->position() - sprite->getLocalBounds().getCenter(); }
	hitbox.value().set_position(pos);
}

void FloatingPart::move(sf::Vector2<float> distance) const { gravitator->set_target_position(gravitator->position() + distance); }

} // namespace fornani::entity
