#include "Breakable.hpp"
#include <cmath>
#include "../entities/player/Player.hpp"
#include "../service/ServiceProvider.hpp"
#include "../level/Map.hpp"
#include "../particle/Effect.hpp"
#include "Breakable.hpp"
#include <algorithm>

namespace world {

Breakable::Breakable(automa::ServiceProvider& svc, sf::Vector2<float> position, int style, int state) : style(style), state(state) {
	collider = shape::Collider();
	collider.physics.position = position;
	collider.sync_components();
	sprite.setTexture(svc.assets.t_breakables);
	sprite.setTextureRect(sf::IntRect{{state * 32, style * 32}, {32, 32}});
	sounds.hit.setBuffer(svc.assets.b_breakable_hit);
	sounds.shatter.setBuffer(svc.assets.shatter_buffer);
}

void Breakable::update(automa::ServiceProvider& svc) {
	if (destroyed()) { return; }
	energy = std::clamp(energy - dampen, 0.f, std::numeric_limits<float>::max());
	if (energy < 0.2f) { energy = 0.f; }
	if (svc.ticker.every_x_ticks(20)) { random_offset = svc.random.random_vector_float(-energy, energy); }
}

void Breakable::handle_collision(shape::Collider& other) const {
	if (destroyed()) { return; }
	other.handle_collider_collision(collider.bounding_box);
}

void Breakable::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam) {
	if (destroyed()) { return; }
	sprite.setPosition(collider.physics.position - cam + random_offset);
	sprite.setTextureRect(sf::IntRect{{style * 32, (state - 1) * 32}, {32, 32}});
	if (svc.greyblock_mode()) {
		collider.render(win, cam);
	} else {
		win.draw(sprite);
	}
}

void Breakable::on_hit(automa::ServiceProvider& svc, world::Map& map, arms::Projectile& proj, int power) {
	if (proj.stats.transcendent) { return; }
	if (destroyed()) { return; }
	if (proj.bounding_box.overlaps(collider.bounding_box)) {
		if (!proj.destruction_initiated()) {
			state -= power;
			energy = hit_energy;
			sounds.hit.play();
		}
		if (destroyed()) {
			map.effects.push_back(entity::Effect(svc, collider.physics.position, {}, 0, 0));
			sounds.shatter.play();
		}
		proj.destroy(false);
	}
}

void Breakable::on_smash(automa::ServiceProvider& svc, world::Map& map, int power) {
	if (destroyed()) { return; }
	state -= power;
	energy = hit_energy;
	sounds.hit.play();
	if (destroyed()) {
		map.effects.push_back(entity::Effect(svc, collider.physics.position, {}, 0, 0));
		sounds.shatter.play();
	}
}

} // namespace world
