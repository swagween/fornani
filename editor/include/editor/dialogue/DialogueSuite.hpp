
#pragma once

#include <SFML/Graphics.hpp>
#include <djson/json.hpp>
#include <editor/dialogue/Node.hpp>
#include <fornani/graphics/Colors.hpp>
#include <fornani/gui/Console.hpp>
#include <string_view>

namespace pi {

class DialogueSuite {
  public:
	DialogueSuite(sf::Font& font, dj::Json const& in, std::string_view host, std::string_view tag);
	void serialize(dj::Json& out);

	void add_message(std::string_view message, NodeType type, int set_index = 0, int message_index = 0);
	void add_code(fornani::gui::MessageCodeType type, int value);
	void update(sf::Vector2f position, bool clicked);
	void render(sf::RenderWindow& win, sf::Vector2f cam);

	void swap_node(Node other);
	void unhover_all();
	void deselect_all();

	// ImGui
	void print_codes();

	[[nodiscard]] auto get_current_node() const -> std::optional<Node>;
	[[nodiscard]] auto get_host() const& -> std::string { return m_host; }
	[[nodiscard]] auto get_tag() const& -> std::string { return m_tag; }
	[[nodiscard]] auto get_current_set() const -> std::size_t { return m_current_set; }
	[[nodiscard]] auto get_current_index() const -> std::size_t { return m_current_index; }
	[[nodiscard]] auto is_any_node_hovered() const -> bool;
	[[nodiscard]] auto is_any_node_selected() const -> bool;

  private:
	std::string m_host{};
	std::string m_tag{};

	NodeType m_current_type{};
	std::size_t m_current_set{};
	std::size_t m_current_index{};

	std::vector<NodeSet> m_suite;
	std::vector<NodeSet> m_responses;
	std::vector<fornani::gui::MessageCode> m_codes{};

	sf::Font* m_font;
};

} // namespace pi
