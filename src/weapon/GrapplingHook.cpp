#include "GrapplingHook.hpp"
#include "../setup/ServiceLocator.hpp"
#include "../service/ServiceProvider.hpp"
#include "../utils/Math.hpp"

namespace arms {
GrapplingHook::GrapplingHook(automa::ServiceProvider& svc) {
	rope.setTexture(svc.assets.t_rope);
	rope.setTextureRect(sf::IntRect({0, 0}, {6, 6}));
}
void GrapplingHook::update(automa::ServiceProvider& svc) {

	if (grapple_flags.test(GrappleState::probing)) {
		spring.set_bob(svc::playerLocator.get().equipped_weapon().barrel_point);
		svc::soundboardLocator.get().flags.weapon.set(audio::Weapon::hook_probe);
	}
	if (grapple_flags.test(GrappleState::anchored)) {
		spring.update(svc);
	}

	if (grapple_triggers.test(arms::GrappleTriggers::released)) {
		svc::playerLocator.get().collider.physics.acceleration = spring.variables.physics.acceleration;
		svc::playerLocator.get().collider.physics.velocity = spring.variables.physics.velocity;
		spring.variables = {};
		spring.set_bob(spring.get_anchor());
		spring.variables.physics.position = spring.get_bob();
		spring.set_anchor(svc::playerLocator.get().apparent_position);
		grapple_triggers.reset(arms::GrappleTriggers::released);
	}
	if (grapple_flags.test(arms::GrappleState::snaking)) {
		spring.set_rest_length(-80);
		spring.set_anchor(svc::playerLocator.get().apparent_position);
		spring.update(svc);
	}
}

void GrapplingHook::break_free() {
	spring.set_force(.2f);
	grapple_flags.reset(arms::GrappleState::anchored);
	grapple_triggers.set(arms::GrappleTriggers::released);
	grapple_flags.set(arms::GrappleState::snaking);
	svc::playerLocator.get().controller.release_hook();
}

void GrapplingHook::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float>& campos) {
	if (svc.debug_flags.test(automa::DebugFlags::greyblock_mode)) {
		spring.render(win, campos);
	} else {
		float distance = util::magnitude(svc::playerLocator.get().collider.physics.position - spring.get_anchor());
		spring.num_links = distance / 20;
		spring.num_links = 10;
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