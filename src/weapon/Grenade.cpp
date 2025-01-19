#include "Grenade.hpp"
#include "../entities/player/Player.hpp"
#include "../service/ServiceProvider.hpp"
#include "../level/Map.hpp"
#include "../../include/fornani/utils/Math.hpp"

namespace arms {

Grenade::Grenade(automa::ServiceProvider& svc, sf::Vector2<float> position, dir::Direction direction) : Collider({16.f, 16.f}), sprite{svc.assets.t_grenade} {
	switch (direction.inter) {
	case dir::Inter::northeast: physics.acceleration = {60.f, -100.f}; break;
	case dir::Inter::east: physics.acceleration = {100.f, -20.f}; break;
	case dir::Inter::southeast: physics.acceleration = {60.f, 0.f}; break;
	case dir::Inter::south: physics.acceleration = {0.f, 0.f}; break;
	case dir::Inter::southwest: physics.acceleration = {-60.f, 0.f}; break;
	case dir::Inter::west: physics.acceleration = {-100.f, -20.f}; break;
	case dir::Inter::northwest: physics.acceleration = {-60.f, -100.f}; break;
	case dir::Inter::north: physics.acceleration = {0.f, -200.f}; break;
	}

	physics.elasticity = 0.8f;
	physics.set_global_friction(0.998f);
	stats.GRAV = 8.8f;

	drawbox.setFillColor(sf::Color::Transparent);
	drawbox.setOutlineColor(sf::Color::White);
	drawbox.setOutlineThickness(-1);
	blast_indicator.setFillColor(sf::Color::Transparent);
	blast_indicator.setOutlineColor(sf::Color::Transparent);
	blast_indicator.setOutlineThickness(-2);
	blast_indicator.setRadius(3 * svc.constants.cell_size);
	blast_indicator.setOrigin({blast_indicator.getRadius(), blast_indicator.getRadius()});

	sensor.bounds.setRadius(3 * svc.constants.cell_size);
	sensor.bounds.setRadius(3 * svc.constants.cell_size);

	Entity::dimensions = Collider::bounding_box.dimensions;
	sprite_dimensions = {16, 22};
	Entity::sprite_offset = {8, 6};
	drawbox.setSize({Entity::dimensions});
	physics.position = position;
	sync_components();
	sprite.setOrigin(sf::Vector2<float>{sprite_dimensions} * 0.5f);
	animation.set_params({0, 7, 40, -1});
	detonator.start(detonation_time);
}

void Grenade::update(automa::ServiceProvider& svc, player::Player& player, world::Map& map) {
	if (detonator.is_complete()) {
		sf::Vector2<float> explosion_position = sensor.bounds.getPosition() - sf::Vector2<float>{sensor.bounds.getRadius(), sensor.bounds.getRadius()};
		map.effects.push_back(entity::Effect(svc, explosion_position, {}, 1, 3));
		svc.soundboard.flags.frdog.set(audio::Frdog::death);
		grenade_flags.set(GrenadeFlags::detonated);
	}
	Collider::update(svc);
	detect_map_collision(map);
	if (physics.elastic_collision()) { svc.soundboard.flags.console.set(audio::Console::next); }
	reset();
	reset_ground_flags();
	physics.acceleration = {};

	sensor.bounds.setPosition(physics.position + Collider::bounding_box.dimensions * 0.5f);

	detonator.update();
	animation.update();
	if (detonator.get_cooldown() % 80 == 0 && animation.params.framerate > 8) { animation.params.framerate -= 5; }
	auto dampen = 0.003f;
	auto sign = physics.velocity.x > 0.f ? -1 : 1;
	sprite.setRotation(sf::degrees((detonator.get_cooldown() * detonator.get_cooldown()) * dampen * sign));
	drawbox.setPosition(physics.position);
}

void Grenade::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam) {
	auto u = 0;
	auto v = animation.get_frame() * sprite_dimensions.y;
	sprite.setTextureRect(sf::IntRect({u, v}, {sprite_dimensions}));
	sprite.setPosition(physics.position + Entity::sprite_offset - cam);
	drawbox.setPosition(physics.position - cam);
	blast_indicator.setPosition(sensor.bounds.getPosition() - cam);
	if(svc.greyblock_mode()) {
		win.draw(drawbox);
	} else {
		if (!detonator.is_complete()) { win.draw(sprite); }
	}

	// blast radius fade-in effect
	float portion = 3.f;
	float start = (float)detonation_time / portion;
	float current = (float)detonator.get_cooldown();
	if (current < start) {
		float timer = (start - current) / start;
		auto alpha = static_cast<uint8_t>(std::lerp(0, 255, timer));
		blast_indicator.setOutlineColor(sf::Color{254, 252, 216, alpha});
		win.draw(blast_indicator);
	}
}

} // namespace arms