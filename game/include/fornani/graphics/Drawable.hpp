
#pragma once

#include <SFML/Graphics.hpp>
#include <string_view>
#include <utility>
#include "fornani/io/Logger.hpp"
#include "fornani/utils/Polymorphic.hpp"

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani {
class Drawable : public sf::Drawable, Polymorphic {
	friend class Animatable;

  public:
	Drawable(automa::ServiceProvider& svc, std::string_view label);
	void draw(sf::RenderTarget& target, sf::RenderStates states = sf::RenderStates::Default) const override;

	void set_texture_rect(sf::IntRect const to) { m_sprite.setTextureRect(to); }
	void set_position(sf::Vector2f const to) { m_sprite.setPosition(to); }
	void set_origin(sf::Vector2f const to) { m_sprite.setOrigin(to); }
	void set_scale(sf::Vector2f const to) { m_sprite.setScale(to); }
	void rotate(sf::Angle const rotation) { m_sprite.rotate(rotation); }

	void scale(sf::Vector2f const to) { m_sprite.scale(to); }
	void center(sf::Vector2f offset = {}) { set_origin(m_sprite.getLocalBounds().getCenter() + offset); }
	void flip(std::pair<bool, bool> orientation = {true, false});

	[[nodiscard]] auto get_scale() const -> sf::Vector2f { return m_sprite.getScale(); }
	[[nodiscard]] auto get_window_position() const -> sf::Vector2f { return m_sprite.getPosition(); }

  protected:
	void set_texture(sf::Texture const& to) { m_sprite.setTexture(to); }
	void set_sprite(sf::Sprite const& sprite) { m_sprite = sprite; }
	io::Logger m_logger{"Rendering"};

  private:
	sf::Sprite m_sprite;
};

} // namespace fornani
