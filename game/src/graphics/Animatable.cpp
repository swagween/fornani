
#include "fornani/graphics/Animatable.hpp"
#include "fornani/graphics/Drawable.hpp"
#include "fornani/service/ServiceProvider.hpp"

namespace fornani {

fornani::Animatable::Animatable(automa::ServiceProvider& svc, std::string_view label, sf::Vector2i dimensions) : Drawable(svc, label), m_dimensions{dimensions} { set_texture_rect(sf::IntRect{{}, m_dimensions}); }

void Animatable::tick() {
	animation.update();
	auto u = m_channel * m_dimensions.x;
	auto v = animation.get_frame() * m_dimensions.y;
	set_texture_rect(sf::IntRect{{u, v}, m_dimensions});
}

} // namespace fornani
