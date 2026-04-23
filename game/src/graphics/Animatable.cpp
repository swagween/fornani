
#include "fornani/graphics/Animatable.hpp"
#include "fornani/graphics/Drawable.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/utils/Random.hpp"

namespace fornani {

Animatable::Animatable(automa::ServiceProvider& svc, std::string_view label, sf::Vector2i dimensions) : Drawable(svc, label), m_dimensions{dimensions} { set_texture_rect(sf::IntRect{{}, m_dimensions}); }

void Animatable::push_animation(std::string_view label, anim::Parameters params) {
	if (p_animations.empty()) { m_root_animation = label.data(); }
	p_animations.insert({label.data(), params});
}

void Animatable::push_and_set_animation(std::string_view label, anim::Parameters params) {
	push_animation(label, params);
	set_animation(label);
}

void Animatable::set_animation(std::string_view to) {
	if (!p_animations.contains(to)) { return; }
	auto it = p_animations.find(to);
	if (it != p_animations.end()) {
		set_parameters(it->second);
		m_current = to.data();
	}
}

void Animatable::set_channel(int to) {
	m_channel = to;
	set_rect();
}

void Animatable::set_frame(int to) {
	animation.set_frame(to);
	set_rect();
}

void Animatable::tick() {
	animation.update();
	set_rect();
}

void Animatable::check_for_switch() {
	if (animation.is_complete()) { set_animation(m_root_animation); }
}

void Animatable::random_start() {
	if (animation.params.duration > 1) { animation.frame.set(random::random_range(0, animation.params.duration - 1)); }
}

void Animatable::random_frame_start() { animation.frame_timer.randomize(); }

void Animatable::set_rect() {
	auto u = m_channel * m_dimensions.x;
	auto v = animation.get_frame() * m_dimensions.y;
	set_texture_rect(sf::IntRect{{u, v}, m_dimensions});
}

} // namespace fornani
