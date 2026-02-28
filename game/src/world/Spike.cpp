
#include "fornani/world/Spike.hpp"
#include "fornani/entities/player/Player.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/utils/Random.hpp"
#include "fornani/world/Map.hpp"

namespace fornani::world {

Spike::Spike(automa::ServiceProvider& svc, sf::Texture const& texture, sf::Vector2f position, NeighborSet neighbors, sf::Vector2f size, int style, bool random)
	: Animatable{svc, is_small() ? "spike" : "big_spike", is_small() ? sf::Vector2i{16, 16} : sf::Vector2i{96, 96}}, size(size), hitbox(is_small() ? size * 28.f : size * 24.f),
	  grid_position{is_small() ? position : position + constants::f_resolution_vec}, facing{neighbors.as_direction()}, collider{constants::f_cell_vec} {
	if (random) { attributes.set(SpikeAttributes::random); }
	if (!is_small()) { attributes.set(SpikeAttributes::soft_reset); }
	center();
	if (attributes.test(SpikeAttributes::random) && is_small()) {
		if (random::percent_chance(50)) { scale({-1.f, 1.f}); }
	}
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
	auto collider_offset = facing.as_vector() * factor;
	collider.physics.position = position + collider_offset;
	collider.sync_components();
	collider.fix();
	set_shape(neighbors);
	is_small() ? set_texture_rect(sf::IntRect{{240, 496}, constants::i_resolution_vec}) : set_texture_rect(sf::IntRect{{96 * static_cast<int>(m_shape), 96 * style}, {96, 96}});
	push_and_set_animation("basic", {style, 1, 24, -1});
}

void Spike::set_shape(NeighborSet neighbors) {
	if (neighbors.get_count() < 2) { m_shape = SpikeShape::peak; }
	if (neighbors.get_count() == 2) { m_shape = SpikeShape::corner; }
	if (neighbors.get_count() > 2) { m_shape = SpikeShape::valley; }
	rotate(neighbors.get_rotation());
}

void Spike::update(automa::ServiceProvider& svc, player::Player& player, world::Map& map) {
	if (!is_small()) { set_channel(static_cast<int>(m_shape)); }
	tick();
	collider.update(svc);
	if (!player.is_dead()) { handle_collision(player.get_collider()); }
	if (attributes.test(SpikeAttributes::soft_reset)) {
		if (map.transition.is(graphics::TransitionState::black)) { player.controller.unrestrict(); }
		if (soft_reset && map.transition.is(graphics::TransitionState::black)) {
			player.set_position(map.last_checkpoint());
			player.get_collider().physics.zero();
			player.controller.prevent_movement();
			player.controller.restrict_movement();
			map.transition.end();
			soft_reset = false;
		}
		if (player.hurtbox.overlaps(hitbox) && map.transition.is(graphics::TransitionState::inactive) && !player.invincible() && !player.is_dead()) {
			player.hurt();
			player.freeze_position();
			player.shake_sprite();
			soft_reset = true;
			map.transition.start();
		}
	} else {
		if (player.hurtbox.overlaps(hitbox)) { player.hurt(); }
	}
}

void Spike::handle_collision(shape::Collider& other) const {
	if (attributes.test(SpikeAttributes::no_collision)) { return; }
	other.handle_collider_collision(collider);
}

void Spike::render(automa::ServiceProvider& svc, sf::RenderWindow& win, std::optional<LightShader>& shader, std::optional<Palette>& palette, sf::Vector2f cam) {
	auto tweak = is_small() ? constants::f_resolution_vec : facing.as_vector() * 32.f + sf::Vector2f{};
	set_position(grid_position + tweak - cam);
	if (shader && palette) {
		shader->submit(win, palette.value(), get_sprite());
	} else {
		win.draw(*this);
	}
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
