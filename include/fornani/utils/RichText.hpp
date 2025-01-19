#include <../../../out/vs22-internal/_deps/sfml-src/include/SFML/Graphics.hpp>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace util {

class RichText : public sf::Drawable, public sf::Transformable {
  public:
	/// @brief Append some text to the list of segments.
	void add_text(std::string_view text, sf::Font const& font, unsigned int size, sf::Color color = sf::Color::White) {
		sf::Text textSegment(font, std::string(text), size);
		textSegment.setFillColor(color);
		textSegment.setPosition({m_current_offset, 0.0f});
		m_current_offset += textSegment.getGlobalBounds().size.x;
		m_segments.emplace_back(textSegment);
	}

	/// @brief Append a sprite to the list of segments. Normally used to display icons inline with text.
	void add_sprite(sf::Sprite sprite) {
		sprite.setPosition({m_current_offset, 0.0f});
		m_current_offset += sprite.getGlobalBounds().size.x;
		m_segments.emplace_back(sprite);
	}

	/// @brief Append some space. It will be placed after the last segment added.
	void add_spacing(float spacing) { m_current_offset += spacing; }

	/// @brief Remove all segments.
	void clear_segments() {
		m_segments.clear();
		m_current_offset = 0.f;
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

	float m_current_offset = 0.f;
	std::vector<Segment> m_segments{};
};

} // namespace util