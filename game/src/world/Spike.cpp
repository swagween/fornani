#include "fornani/world/Spike.hpp"

#include <cmath>
#include "fornani/entities/player/Player.hpp"
#include "fornani/particle/Effect.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/world/Map.hpp"

#include "fornani/utils/Random.hpp"

namespace fornani::world {

Spike::Spike(automa::ServiceProvider& svc, sf::Texture const& texture, sf::Vector2f position, sf::Vector2<int> direction, sf::Vector2f size, int style, bool random)
	: size(size), hitbox(is_small() ? size * 28.f : size * 24.f), sprite{texture}, grid_position{is_small() ? position : position + constants::f_resolution_vec}, facing{-direction, true}, collider{constants::f_cell_vec} {
	if (random) { attributes.set(SpikeAttributes::random); }
	if (!is_small()) { attributes.set(SpikeAttributes::soft_reset); }
	if (facing.left_or_right() && facing.up_or_down() && is_small()) { facing.neutralize_lr(); } // small spikes prefer to face up or down
	is_small() ? sprite.setTextureRect(sf::IntRect{{240, 496}, constants::i_resolution_vec}) : sprite.setTextureRect(sf::IntRect{{0, 64 * style}, {96, 64}});
	sprite.setOrigin(size * 8.f);
	sprite.setScale(constants::f_scale_vec);
	if (attributes.test(SpikeAttributes::random)) {
		if (random::percent_chance(50)) { sprite.scale({-1.f, 1.f}); }
	}
	if (facing.left()) { sprite.rotate(sf::degrees(-90)); }
	if (facing.right()) { sprite.rotate(sf::degrees(90)); }
	if (facing.down()) { sprite.rotate(sf::degrees(180)); }
	auto x_off = is_small() ? -2.f : 56.f;
	auto y_off = is_small() ? -2.f : 32.f;
	offset = sf::Vector2f{x_off, y_off};
	if (facing.left_or_right()) {
		offset = {y_off, x_off};
		auto rotation = sf::Vector2f{hitbox.get_dimensions().y, hitbox.get_dimensions().x};
		hitbox.set_dimensions(rotation);
	}
	is_small() ? hitbox.set_position(position - offset) : hitbox.set_position(position - offset);
	auto factor = -24.f;
	auto collider_offset = facing.get_vector() * factor;
	collider.physics.position = position + collider_offset;
	collider.sync_components();
	collider.fix();
}

void Spike::update(automa::ServiceProvider& svc, player::Player& player, world::Map& map) {
	collider.update(svc);
	handle_collision(player.collider);
	if (attributes.test(SpikeAttributes::soft_reset)) {
		if (map.soft_reset.is_done()) { player.controller.unrestrict(); }
		if (soft_reset && map.soft_reset.is_done()) {
			player.set_position(map.last_checkpoint());
			player.collider.physics.zero();
			player.controller.prevent_movement();
			player.controller.restrict_movement();
			map.soft_reset.end();
			soft_reset = false;
		}
		if (player.hurtbox.overlaps(hitbox) && map.soft_reset.not_started() && !player.invincible()) {
			player.hurt();
			player.freeze_position();
			player.shake_sprite();
			if (!player.is_dead()) {
				soft_reset = true;
				map.soft_reset.start();
			}
		}
	} else {
		if (player.hurtbox.overlaps(hitbox)) { player.hurt(); }
	}
}

void Spike::handle_collision(shape::Collider& other) const {
	if (attributes.test(SpikeAttributes::no_collision)) { return; }
	other.handle_collider_collision(collider);
}

void Spike::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {
	auto tweak = is_small() ? constants::f_resolution_vec : sf::Vector2f{};
	sprite.setPosition(grid_position + tweak - cam);
	win.draw(sprite);
	if (svc.greyblock_mode()) {
		collider.render(win, cam);
		drawbox.setPosition(hitbox.get_position() - cam);
		drawbox.setSize(hitbox.get_dimensions());
		drawbox.setOutlineColor(colors::ui_white);
		drawbox.setOutlineThickness(-1.f);
		drawbox.setFillColor(sf::Color{249, 12, 48, 64});
		win.draw(drawbox);
	}
}

} // namespace fornani::world
