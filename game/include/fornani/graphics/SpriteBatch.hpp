
#pragma once

#include <SFML/Graphics.hpp>
#include <fornani/utils/BitFlags.hpp>

namespace fornani {

enum class SpriteFlip { none, horizontal, vertical };
enum class RenderLayer { background, scenery, background_entities, player, platforms, projectiles, atmosphere, middleground, particles, foreground_entities, effects, foreground, hud };

struct RenderCommand {
	sf::Texture const* texture{};
	sf::FloatRect dest{};
	sf::IntRect uv{};
	sf::Color color{};
	util::BitFlags<SpriteFlip> flip{};
	RenderLayer layer{};
};

class SpriteBatch : public sf::Drawable, public sf::Transformable {
  public:
	void clear() { vertices.clear(); }

	void setTexture(sf::Texture const& tex) { texture = &tex; }

	void add(sf::FloatRect const& dest, sf::IntRect const& uv, sf::Color color = sf::Color::White, util::BitFlags<SpriteFlip> flip = {}) {

		sf::Vertex quad[4];

		float x = dest.position.x;
		float y = dest.position.y;
		float w = dest.size.x;
		float h = dest.size.y;

		float u0 = static_cast<float>(uv.position.x);
		float v0 = static_cast<float>(uv.position.y);
		float u1 = static_cast<float>(uv.position.x + uv.size.x);
		float v1 = static_cast<float>(uv.position.y + uv.size.y);

		if (flip.test(SpriteFlip::horizontal)) std::swap(u0, u1);
		if (flip.test(SpriteFlip::vertical)) std::swap(v0, v1);

		quad[0].position = {x, y};
		quad[1].position = {x + w, y};
		quad[2].position = {x + w, y + h};
		quad[3].position = {x, y + h};

		quad[0].texCoords = {u0, v0};
		quad[1].texCoords = {u1, v0};
		quad[2].texCoords = {u1, v1};
		quad[3].texCoords = {u0, v1};

		for (auto& v : quad) v.color = color;

		vertices.append(quad[0]);
		vertices.append(quad[1]);
		vertices.append(quad[2]);

		vertices.append(quad[0]);
		vertices.append(quad[2]);
		vertices.append(quad[3]);
	}

	[[nodiscard]] auto is_empty() const -> bool { return vertices.getVertexCount() == 0; }

  private:
	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override {

		states.transform *= getTransform();
		states.texture = texture;

		target.draw(vertices, states);
	}

  private:
	util::BitFlags<SpriteFlip> m_flip{};
	sf::VertexArray vertices{sf::PrimitiveType::Triangles};
	sf::Texture const* texture{};
};

} // namespace fornani
