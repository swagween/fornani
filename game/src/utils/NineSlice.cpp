#include "fornani/utils/NineSlice.hpp"
#include "fornani/service/ServiceProvider.hpp"

namespace fornani::util {

NineSlice::NineSlice(automa::ServiceProvider& svc, sf::Texture& tex, sf::Vector2i edge, sf::Vector2i corner) : m_sprite{tex}, m_edge_dimensions{edge}, m_corner_dimensions{corner} { m_sprite.setScale(svc.constants.texture_scale); }

void NineSlice::render(sf::RenderWindow& win, sf::Vector2f cam) {
	m_sprite.setTextureRect(sf::IntRect{{}, m_corner_dimensions});
	m_sprite.setOrigin(get_local_center());
	m_sprite.setPosition(m_physics.position - cam);
	win.draw(m_sprite);
}

} // namespace fornani::util
