
#include "fornani/world/Spike.hpp"
#include "fornani/entities/player/Player.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/utils/Random.hpp"
#include "fornani/world/Map.hpp"

namespace fornani::world {

Spike::Spike(automa::ServiceProvider& svc, sf::Texture const& texture, sf::Vector2f position, NeighborSet neighbors, int style, bool random)
	: Animatable{svc, "spike", constants::i_resolution_vec}, hitbox({28.f, 28.f}), facing{neighbors.get_direction_via(special_index_v + 62)}, grid_position{position}, collider{constants::f_cell_vec} {
	center();
	rotate(facing.as_angle());
	if (random) {
		if (random::percent_chance(50)) { scale({-1.f, 1.f}); }
	}
	auto x_off = -2.f;
	auto y_off = -2.f;
	offset = sf::Vector2f{x_off, y_off};
	if (facing.left_or_right()) {
		offset = {y_off, x_off};
		auto rotation = sf::Vector2f{hitbox.get_dimensions().y, hitbox.get_dimensions().x};
		hitbox.set_dimensions(rotation);
	}
	hitbox.set_position(position - offset);
	auto factor = -24.f;
	auto collider_offset = facing.as_vector() * factor;
	collider.physics.position = position + collider_offset;
	collider.sync_components();
	collider.fix();
	push_and_set_animation("basic", {style, 1, 24, -1});
}

void Spike::update(automa::ServiceProvider& svc, player::Player& player, world::Map& map) {
	tick();
	collider.update(svc);
	if (!player.is_dead()) { handle_collision(player.get_collider()); }
	if (player.hurtbox.overlaps(hitbox)) { player.hurt(); }
}

void Spike::handle_collision(shape::Collider& other) const { other.handle_collider_collision(collider); }

void Spike::render(automa::ServiceProvider& svc, sf::RenderWindow& win, std::optional<LightShader>& shader, std::optional<Palette>& palette, sf::Vector2f cam) {
	auto tweak = constants::f_resolution_vec;
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
