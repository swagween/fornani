
#include "fornani/graphics/Drawable.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/utils/Constants.hpp"

namespace fornani {

Drawable::Drawable(automa::ServiceProvider& svc, std::string_view label) : m_sprite{svc.assets.get_texture(label.data())} { m_sprite.setScale(constants::f_scale_vec); }

void Drawable::draw(sf::RenderTarget& target, sf::RenderStates states) const { target.draw(m_sprite); }

void Drawable::flip(std::pair<bool, bool> orientation) {
	if (orientation.first) { m_sprite.scale({-1.f, 1.f}); }
	if (orientation.second) { m_sprite.scale({1.f, -1.f}); }
}

} // namespace fornani
