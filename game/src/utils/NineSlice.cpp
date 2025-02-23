#include "fornani/utils/NineSlice.hpp"
#include "fornani/service/ServiceProvider.hpp"

namespace fornani::util {

NineSlice::NineSlice(automa::ServiceProvider& svc, sf::Texture& tex, sf::Vector2i corner, sf::Vector2i edge) : m_sprite{tex}, m_corner_dimensions{corner}, m_edge_dimensions{edge}, m_native_scale{svc.constants.texture_scale} {}

void NineSlice::render(sf::RenderWindow& win, sf::Vector2f cam) {
	auto half_edge{m_dimensions * 0.5f};
	auto f_dim{sf::Vector2f{m_dimensions}};
	auto f_corner{sf::Vector2f{static_cast<float>(m_corner_dimensions.x), static_cast<float>(m_corner_dimensions.y)}};

	m_sprite.setScale(m_native_scale); // refresh scale

	// corners
	m_sprite.setOrigin(f_corner);
	m_sprite.setTextureRect(sf::IntRect{{}, m_corner_dimensions}); // top left
	m_sprite.setPosition(m_physics.position - cam + m_render_offset);
	win.draw(m_sprite);
	m_sprite.setOrigin({0.f, f_corner.y});
	m_sprite.setTextureRect(sf::IntRect{{m_corner_dimensions.x + m_edge_dimensions.x, 0}, m_corner_dimensions}); // top right
	m_sprite.setPosition(m_physics.position + sf::Vector2f{f_dim.x, 0.f} - cam + m_render_offset);
	win.draw(m_sprite);
	m_sprite.setOrigin({f_corner.x, 0.f});
	m_sprite.setTextureRect(sf::IntRect{{0, m_corner_dimensions.y + m_edge_dimensions.y}, m_corner_dimensions}); // bottom left
	m_sprite.setPosition(m_physics.position + sf::Vector2f{0.f, f_dim.y} - cam + m_render_offset);
	win.draw(m_sprite);
	m_sprite.setOrigin({});
	m_sprite.setTextureRect(sf::IntRect{{m_corner_dimensions + m_edge_dimensions}, m_corner_dimensions}); // bottom right
	m_sprite.setPosition(m_physics.position + f_dim - cam + m_render_offset);
	win.draw(m_sprite);

	// edges
	m_sprite.setOrigin({0.f, f_corner.y});
	m_sprite.setScale({m_dimensions.x, m_native_scale.y});
	m_sprite.setTextureRect(sf::IntRect{{m_corner_dimensions.x, 0}, {m_edge_dimensions.x, m_corner_dimensions.y}}); // top middle
	m_sprite.setPosition(m_physics.position - cam + m_render_offset);
	win.draw(m_sprite);
	m_sprite.setOrigin({});
	m_sprite.setScale({m_dimensions.x, m_native_scale.y});
	m_sprite.setTextureRect(sf::IntRect{{m_corner_dimensions.x, m_corner_dimensions.y + m_edge_dimensions.y}, {m_edge_dimensions.x, m_corner_dimensions.y}}); // bottom middle
	m_sprite.setPosition(m_physics.position + sf::Vector2f{0.f, f_dim.y} - cam + m_render_offset);
	win.draw(m_sprite);
	m_sprite.setOrigin({f_corner.x, 0.f});
	m_sprite.setScale({m_native_scale.x, m_dimensions.y});
	m_sprite.setTextureRect(sf::IntRect{{0, m_corner_dimensions.y}, {m_corner_dimensions.x, m_edge_dimensions.y}}); // middle left
	m_sprite.setPosition(m_physics.position - cam + m_render_offset);
	win.draw(m_sprite);
	m_sprite.setOrigin({});
	m_sprite.setScale({m_native_scale.x, m_dimensions.y});
	m_sprite.setTextureRect(sf::IntRect{{m_corner_dimensions.x + m_edge_dimensions.x, m_corner_dimensions.y}, {m_corner_dimensions.x, m_edge_dimensions.y}}); // middle right
	m_sprite.setPosition(m_physics.position + sf::Vector2f{f_dim.x, 0.f} - cam + m_render_offset);
	win.draw(m_sprite);
	m_sprite.setOrigin({});
	m_sprite.setScale({m_dimensions.x, m_dimensions.y});
	m_sprite.setTextureRect(sf::IntRect{m_corner_dimensions, m_edge_dimensions}); // middle
	m_sprite.setPosition(m_physics.position - cam + m_render_offset);
	win.draw(m_sprite);
}

} // namespace fornani::util
