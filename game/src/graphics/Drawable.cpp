
#include "fornani/graphics/Drawable.hpp"
#include <fornani/core/Debug.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include "fornani/utils/Constants.hpp"

namespace fornani {

Drawable::Drawable(automa::ServiceProvider& svc, std::string_view label) : m_sprite{svc.assets.get_texture(label.data())} {
	m_sprite.setScale(constants::f_scale_vec);
	m_shape.setSize(m_sprite.getLocalBounds().size);
	m_shape.setOutlineColor(colors::goldenrod);
	m_shape.setOutlineThickness(-1.f);
	m_shape.setFillColor(colors::transparent);
	m_shape.setScale(m_sprite.getScale());
	m_shape.setOrigin(m_sprite.getOrigin());
	m_shape.setPosition(m_sprite.getPosition());
}

void Drawable::draw(sf::RenderTarget& target, sf::RenderStates states) const {
	switch (debug::mode) {
	case debug::PresentationMode::production:
		target.draw(m_sprite);
		++debug::draw_calls;
		break;
	case debug::PresentationMode::debug: target.draw(m_shape); break;
	case debug::PresentationMode::greyblock: break;
	}
}

void Drawable::flip(std::pair<bool, bool> orientation) {
	if (orientation.first) { m_sprite.scale({-1.f, 1.f}); }
	if (orientation.second) { m_sprite.scale({1.f, -1.f}); }
}

} // namespace fornani
