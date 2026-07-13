
#include <fornani/entities/Mobile.hpp>
#include <fornani/entities/player/Player.hpp>
#include <fornani/service/ServiceProvider.hpp>

namespace fornani {

Mobile::Mobile(automa::ServiceProvider& svc, world::Map& map, std::string_view label, sf::Vector2i dimensions, bool include_collider) : p_animatable(svc, label, dimensions) {
	owned_collider.emplace(map, sf::Vector2f{dimensions});
	collider = *owned_collider;
}

Mobile::Mobile(automa::ServiceProvider& svc, std::string_view label, sf::Vector2i dimensions) : p_animatable(svc, label, dimensions) {
	owned_collider.reset();
	collider.reset();
}

void Mobile::register_collider(world::Map& map, sf::Vector2f dimensions) {
	owned_collider.emplace(map, dimensions);
	collider = *owned_collider;
}

void Mobile::face_player(player::Player& player) { directions.desired.set((player.get_collider().get_center().x < get_collider().get_center().x) ? LNR::left : LNR::right); }

void Mobile::set_direction(SimpleDirection to) {
	directions.desired = Direction{to};
	directions.actual = Direction{to};
	directions.movement = Direction{to};
	p_animatable.set_scale(constants::f_scale_vec.componentWiseMul({-to.as_float(), 1.f}));
}

void Mobile::set_desired_direction(SimpleDirection to) { directions.desired = Direction{to}; }

bool Mobile::player_behind(player::Player& player) const { return player.get_center().x < get_collider().get_center().x; }

void Mobile::post_update(automa::ServiceProvider& svc, world::Map& map, player::Player& player, bool tick) {
	if (p_flags.consume(MobileState::flip)) {
		if (directions.desired.lnr != directions.actual.lnr) { p_animatable.flip(); }
		directions.desired.unlock();
		directions.actual = directions.desired;
	}
	if (tick) { p_animatable.tick(); }

	auto it = p_sounds.find(p_animatable.get_animation_tag());
	if (it != p_sounds.end()) {
		if (p_animatable.animation.get_frame_count() == it->second.frame && p_animatable.animation.keyframe_started()) { svc.soundboard.play_sound(it->second.tag, get_collider().get_center()); }
	}
}

anim::Parameters const& Mobile::get_params(std::string const& key) { return p_animatable.has_animation(key) ? p_animatable.get_params(key) : p_animatable.get_params("idle"); }

} // namespace fornani
