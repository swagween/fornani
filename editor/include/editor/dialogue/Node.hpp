
#pragma once

#include <SFML/Graphics.hpp>
#include <fornani/graphics/Colors.hpp>
#include <optional>
#include <string_view>

namespace pi {

enum class NodeType : std::uint8_t { suite, response };

class Node {
  public:
	Node(sf::Font const& font, std::string_view message, NodeType type);
	Node& operator=(Node const& other);

	void update(sf::Vector2f position);
	void render(sf::RenderWindow& win, sf::Vector2f cam);
	void set_position(sf::Vector2f to) { m_position = to; }
	void set_hovered(bool to) { m_hovered = to; }
	void set_message(std::string const& to) { m_text->setString(to); }

	[[nodiscard]] auto get_message() const -> std::string { return m_text ? m_text->getString() : ""; }
	[[nodiscard]] auto get_position() const -> sf::Vector2f { return m_position; }
	[[nodiscard]] auto get_size() const -> sf::Vector2f { return box.getLocalBounds().size; }
	[[nodiscard]] auto get_type() const -> NodeType { return m_type; }
	[[nodiscard]] auto is_hovered() const -> bool { return m_hovered; }

  private:
	sf::Vector2f m_position{};
	std::optional<sf::Text> m_text;
	sf::RectangleShape box{};
	NodeType m_type{};
	bool m_hovered{};
};

struct NodeSet {
	NodeSet(NodeType type) : type{type} {}
	void push_back(Node in) { nodes.push_back(in); }
	void update(sf::Vector2f position);
	void render(sf::RenderWindow& win, sf::Vector2f cam);
	void set_position(sf::Vector2f to) { m_position = to; }
	void unhover_all();

	[[nodiscard]] auto get_size() const -> sf::Vector2f { return m_tab.getLocalBounds().size; }
	[[nodiscard]] auto is(NodeType const comparison) const -> bool { return type == comparison; }

	std::vector<Node> nodes{};
	NodeType type{};

  private:
	sf::RectangleShape m_tab{};
	sf::Vector2f m_position{};
	bool m_hovered{};
};

} // namespace pi
