#include "GrapplingHook.hpp"
#include "../setup/ServiceLocator.hpp"
#include "../utils/Math.hpp"

namespace arms {

void GrapplingHook::update() {

	if (grapple_flags.test(GrappleState::probing)) { spring.set_bob(player.equipped_weapon().barrel_point); }
	if (grapple_flags.test(GrappleState::anchored)) {
		spring.update();
	}

	if (grapple_triggers.test(arms::GrappleTriggers::released)) {
		player.collider.physics.acceleration = spring.variables.physics.acceleration;
		player.collider.physics.velocity = spring.variables.physics.velocity;
		spring.variables = {};
		spring.set_bob(spring.get_anchor());
		spring.variables.physics.position = spring.get_bob();
		spring.set_anchor(player.apparent_position);
		svc.loggerLocator.get().triggers.set(util::Trigger::hook_released);
		grapple_triggers.reset(arms::GrappleTriggers::released);
	}
	if (grapple_flags.test(arms::GrappleState::snaking)) {
		spring.set_rest_length(-80);
		spring.set_anchor(player.apparent_position);
		spring.update();
		svc.loggerLocator.get().states.set(util::State::hook_snaking);
	} else {
		svc.loggerLocator.get().states.reset(util::State::hook_snaking);
	}

	svc.loggerLocator.get().hook_bob_position = spring.get_bob();
	svc.loggerLocator.get().hook_anchor_position = spring.get_anchor();
	svc.loggerLocator.get().hook_physics_position = spring.variables.physics.position;
}

void GrapplingHook::break_free() {
	spring.set_force(.2f);
	grapple_flags.reset(arms::GrappleState::anchored);
	grapple_triggers.set(arms::GrappleTriggers::released);
	grapple_flags.set(arms::GrappleState::snaking);
	player.controller.release_hook();
}

void GrapplingHook::render(sf::RenderWindow& win, sf::Vector2<float>& campos) {
	if (svc.globalBitFlagsLocator.get().test(services::global_flags::greyblock_state)) {
		spring.render(win, campos);
	} else {
		rope.setTexture(svc.assetLocator.get().t_rope);
		rope.setTextureRect(sf::IntRect({0, 0}, {6, 6}));
		float distance = util::magnitude(player.collider.physics.position - spring.get_anchor());
		spring.num_links = distance / 20;
		for (int i = 0; i < spring.num_links; ++i) {
			rope.setPosition(spring.get_rope(i) - campos);
			win.draw(rope);
		}
	}
}


sf::Vector2<float> GrapplingHook::probe_velocity(float speed) {
	sf::Vector2<float> ret{};
	float tweak = speed / 3;

	switch (probe_direction.inter) {
	case dir::Inter::north: ret = {0, -speed}; break;
	case dir::Inter::south: ret = {0, speed}; break;
	case dir::Inter::east: ret = {speed, -tweak}; break;
	case dir::Inter::west: ret = {-speed, -tweak}; break;
	case dir::Inter::northwest: ret = {-speed, -speed}; break;
	case dir::Inter::northeast: ret = {speed, -speed}; break;
	case dir::Inter::southwest: ret = {-speed, tweak}; break;
	case dir::Inter::southeast: ret = {speed, tweak}; break;
	}

	return ret;
}
} // namespace arms