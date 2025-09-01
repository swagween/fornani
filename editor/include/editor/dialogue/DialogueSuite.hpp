
#pragma once

#include <SFML/Graphics.hpp>
#include <djson/json.hpp>
#include <fornani/graphics/Colors.hpp>
#include <fornani/gui/Console.hpp>
#include <string_view>

namespace pi {

class Node {
  public:
	Node(sf::Font const& font, std::string_view message) : m_text{sf::Text{font}} {
		if (m_text) {
			m_text->setString(message.data());
			m_text->setFillColor(fornani::colors::nani_white);
			m_text->setCharacterSize(10);
			box.setSize(m_text->getLocalBounds().size * 1.5f);
		}
		box.setOutlineColor(fornani::colors::blue);
		box.setOutlineThickness(2.f);
		box.setFillColor(fornani::colors::dark_grey);
	}

	void render(sf::RenderWindow& win, sf::Vector2f cam) {
		box.setPosition(m_position - cam);
		win.draw(box);
		if (m_text) {
			m_text->setPosition(m_position - cam);
			win.draw(*m_text);
		}
	}

	void set_position(sf::Vector2f to) { m_position = to; }

	[[nodiscard]] auto get_position() const -> sf::Vector2f { return m_position; }
	[[nodiscard]] auto get_size() const -> sf::Vector2f { return box.getLocalBounds().size; }

  private:
	sf::Vector2f m_position{};
	std::optional<sf::Text> m_text;
	sf::RectangleShape box{};
};

class DialogueSuite {
  public:
	DialogueSuite(sf::Font const& font, dj::Json const& in, std::string_view host, std::string_view tag);
	void serialize(dj::Json& out);

	void add_message(std::string_view message, int set_index = 0, int message_index = 0);

	void render(sf::RenderWindow& win, sf::Vector2f cam);

	[[nodiscard]] auto get_host() const& -> std::string { return m_host; }
	[[nodiscard]] auto get_tag() const& -> std::string { return m_tag; }

  private:
	std::string m_host{};
	std::string m_tag{};

	std::vector<std::vector<Node>> m_nodes;
	std::vector<std::vector<std::string>> m_suite{};
	std::vector<std::vector<std::string>> m_responses{};
	std::vector<std::vector<fornani::gui::MessageCode>> m_codes{};
};

} // namespace pi
