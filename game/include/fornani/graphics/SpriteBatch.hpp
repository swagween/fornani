
#pragma once

#include <SFML/Graphics.hpp>
#include <fornani/utils/BitFlags.hpp>

namespace fornani {

enum class SpriteTransform : std::uint8_t { none, horizontal, vertical, rotate };
enum class RenderLayer : std::uint8_t { background, scenery, background_entities, player, platforms, projectiles, atmosphere, middleground, particles, foreground_entities, effects, foreground, hud };

struct RenderCommand {
	sf::Texture const* texture{};
	sf::FloatRect dest{};
	sf::IntRect uv{};
	sf::Color color{};
	util::BitFlags<SpriteTransform> flip{};
	RenderLayer layer{};
};

class SpriteBatch : public sf::Drawable, public sf::Transformable {
  public:
	void clear() { vertices.clear(); }

	void setTexture(sf::Texture const& tex) { texture = &tex; }
	void add(sf::FloatRect const& dest, sf::IntRect const& uv, sf::Color color = sf::Color::White, util::BitFlags<SpriteTransform> flip = {}) {
		sf::Vertex quad[4];

		float const x = dest.position.x;
		float const y = dest.position.y;
		float const w = dest.size.x;
		float const h = dest.size.y;
		float const u0 = static_cast<float>(uv.position.x);
		float const v0 = static_cast<float>(uv.position.y);
		float const u1 = static_cast<float>(uv.position.x + uv.size.x);
		float const v1 = static_cast<float>(uv.position.y + uv.size.y);

		quad[0].position = {x, y};
		quad[1].position = {x + w, y};
		quad[2].position = {x + w, y + h};
		quad[3].position = {x, y + h};

		sf::Vector2f texcoords[4] = {{u0, v0}, {u1, v0}, {u1, v1}, {u0, v1}};

		// horizontal flip
		if (flip.test(SpriteTransform::horizontal)) {

			std::swap(texcoords[0], texcoords[1]);
			std::swap(texcoords[3], texcoords[2]);
		}

		// vertical flip
		if (flip.test(SpriteTransform::vertical)) {

			std::swap(texcoords[0], texcoords[3]);
			std::swap(texcoords[1], texcoords[2]);
		}

		// 90 degree clockwise rotation
		if (flip.test(SpriteTransform::rotate)) {

			auto const tmp = texcoords[0];

			texcoords[0] = texcoords[3];
			texcoords[3] = texcoords[2];
			texcoords[2] = texcoords[1];
			texcoords[1] = tmp;
		}

		quad[0].texCoords = texcoords[0];
		quad[1].texCoords = texcoords[1];
		quad[2].texCoords = texcoords[2];
		quad[3].texCoords = texcoords[3];

		for (auto& v : quad) { v.color = color; }

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
	util::BitFlags<SpriteTransform> m_flip{};
	sf::VertexArray vertices{sf::PrimitiveType::Triangles};
	sf::Texture const* texture{};
};

} // namespace fornani
