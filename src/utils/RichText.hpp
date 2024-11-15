#include <SFML/Graphics.hpp>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace util {

class RichText : public sf::Drawable, public sf::Transformable {
  public:
	void clearSegments() { m_segments.clear(); }
	// Add a text segment
	void addText(std::string_view text, sf::Font const& font, unsigned int size, sf::Color color = sf::Color::White) {
		sf::Text textSegment(std::string(text), font, size);
		textSegment.setFillColor(color);
		m_segments.emplace_back(textSegment);
	}

	// Add a sprite
	void addSprite(sf::Sprite const& sprite) { m_segments.emplace_back(sprite); }

	// Arrange all segments
	void arrange(float spacing = 5.0f) {
		float xOffset = 0.0f;

		for (auto& segment : m_segments) {
			if (std::holds_alternative<sf::Text>(segment)) {
				sf::Text& text = std::get<sf::Text>(segment);
				text.setPosition(xOffset, 0.0f);
				xOffset += text.getGlobalBounds().width + spacing;
			} else if (std::holds_alternative<sf::Sprite>(segment)) {
				sf::Sprite& sprite = std::get<sf::Sprite>(segment);
				sprite.setPosition(xOffset, 0.0f);
				xOffset += sprite.getGlobalBounds().width + spacing;
			}
		}
	}

	// Draw all segments
	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override {
		states.transform *= getTransform(); // Apply transformations

		for (auto const& segment : m_segments) {
			if (std::holds_alternative<sf::Text>(segment)) {
				target.draw(std::get<sf::Text>(segment), states);
			} else if (std::holds_alternative<sf::Sprite>(segment)) {
				target.draw(std::get<sf::Sprite>(segment), states);
			}
		}
	}

  private:
	// Each segment is either a text or a sprite
	using Segment = std::variant<sf::Text, sf::Sprite>;
	std::vector<Segment> m_segments;
};

} // namespace util