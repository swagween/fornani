#include "fornani/level/Breakable.hpp"
#include <algorithm>
#include <cmath>
#include "fornani/entities/player/Player.hpp"
#include "fornani/level/Map.hpp"
#include "fornani/particle/Effect.hpp"
#include "fornani/service/ServiceProvider.hpp"

#include "fornani/utils/Random.hpp"

namespace fornani::world {

Breakable::Breakable(automa::ServiceProvider& svc, sf::Vector2<float> position, int style, int state) : style(style), state(state), sprite(svc.assets.t_breakables) {
	collider = shape::Collider({32.f, 32.f});
	collider.physics.position = position;
	collider.sync_components();
	sprite.setTextureRect(sf::IntRect{{state * 32, style * 32}, {32, 32}});
}

void Breakable::update(automa::ServiceProvider& svc, player::Player& player) {
	if (destroyed()) { return; }
	energy = std::clamp(energy - dampen, 0.f, std::numeric_limits<float>::max());
	if (energy < 0.2f) { energy = 0.f; }
	if (svc.ticker.every_x_ticks(20)) { random_offset = util::Random::random_vector_float(-energy, energy); }
	handle_collision(player.collider);
}

void Breakable::handle_collision(shape::Collider& other) const {
	if (destroyed()) { return; }
	other.handle_collider_collision(collider);
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

void Breakable::on_hit(automa::ServiceProvider& svc, Map& map, arms::Projectile& proj, int power) {
	if (proj.transcendent()) { return; }
	if (destroyed()) { return; }
	if (!collider.vicinity.overlaps(proj.get_bounding_box())) { return; }
	if (proj.get_bounding_box().overlaps(collider.bounding_box)) {
		if (!proj.destruction_initiated()) {
			state -= power == 1 ? static_cast<int>(proj.get_power()) : power;
			energy = hit_energy;
			svc.soundboard.flags.world.set(audio::World::breakable_hit);
		}
		if (destroyed()) {
			map.effects.push_back(entity::Effect(svc, collider.physics.position, {}, 0, 0));
			svc.soundboard.flags.world.set(audio::World::breakable_shatter);
		}
		proj.destroy(false);
	}
}

void Breakable::on_smash(automa::ServiceProvider& svc, world::Map& map, int power) {
	if (destroyed()) { return; }
	state -= power;
	energy = hit_energy;
	svc.soundboard.flags.world.set(audio::World::breakable_hit);
	if (destroyed()) {
		map.effects.push_back(entity::Effect(svc, collider.physics.position, {}, 0, 0));
		svc.soundboard.flags.world.set(audio::World::breakable_shatter);
	}
}

} // namespace fornani::world
