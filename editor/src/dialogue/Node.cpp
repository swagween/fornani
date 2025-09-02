
#include <editor/dialogue/Node.hpp>

namespace pi {

constexpr auto box_buffer = sf::Vector2f{20.f, 20.f};

Node::Node(sf::Font const& font, std::string_view message, NodeType type) : m_text{sf::Text{font}}, m_type{type} {
	if (m_text) {
		m_text->setString(message.data());
		type == NodeType::suite ? m_text->setFillColor(fornani::colors::pioneer_red) : m_text->setFillColor(fornani::colors::dark_goldenrod);
		m_text->setCharacterSize(16);
		box.setSize(m_text->getLocalBounds().size + box_buffer);
		m_text->setOrigin(m_text->getLocalBounds().getCenter());
	}
	type == NodeType::suite ? box.setOutlineColor(fornani::colors::pioneer_red) : box.setOutlineColor(fornani::colors::dark_goldenrod);
}

Node& Node::operator=(Node const& other) {
	if (this != &other) { // check for self-assignment
		m_text = other.m_text;
		m_position = other.m_position;
		box = other.box;
		m_type = other.m_type;
		m_hovered = other.m_hovered;
	}
	return *this;
}

void Node::update(sf::Vector2f position, bool clicked) {
	m_hovered = box.getGlobalBounds().contains(position);
	if (m_hovered && clicked) { m_selected = true; }
}

void Node::render(sf::RenderWindow& win, sf::Vector2f cam) {
	box.setPosition(m_position - cam);
	m_hovered || m_selected ? box.setFillColor(fornani::colors::pioneer_dark_red) : box.setFillColor(sf::Color::Transparent);
	m_hovered || m_selected ? box.setOutlineThickness(-4.f) : box.setOutlineThickness(-2.f);
	m_type == NodeType::suite ? box.setOutlineColor(fornani::colors::pioneer_red) : box.setOutlineColor(fornani::colors::dark_goldenrod);
	if (m_selected) { box.setOutlineColor(fornani::colors::ui_white); }
	win.draw(box);
	if (m_text) {
		m_text->setPosition(box.getGlobalBounds().getCenter());
		win.draw(*m_text);
	}
}

void NodeSet::update(sf::Vector2f position) {
	m_hovered = m_tab.getGlobalBounds().contains(position);
	if (m_hovered) {
		for (auto& node : nodes) { node.set_hovered(true); }
	}
}

void NodeSet::render(sf::RenderWindow& win, sf::Vector2f cam) {
	sf::Vector2f previous_position{};
	auto buffer = 1.2f;
	type == NodeType::suite ? m_tab.setFillColor(fornani::colors::pioneer_dark_red) : m_tab.setFillColor(fornani::colors::pioneer_dark_red);
	if (m_hovered) { type == NodeType::suite ? m_tab.setFillColor(fornani::colors::pioneer_red) : m_tab.setFillColor(fornani::colors::dark_goldenrod); }
	m_tab.setPosition(m_position + sf::Vector2f{-24.f, 0.f} - cam);
	auto tab_size = sf::Vector2f{16.f, 0.f};
	for (auto& node : nodes) {
		node.set_position(previous_position);
		previous_position = node.get_position() + sf::Vector2f{0.f, node.get_size().y * buffer};
		node.render(win, cam - m_position);
		tab_size.y += node.get_size().y * buffer;
	}
	m_tab.setSize(tab_size);
	win.draw(m_tab);
}

void NodeSet::unhover_all() {
	for (auto& node : nodes) { node.set_hovered(false); }
}

void NodeSet::deselect_all() {
	for (auto& node : nodes) { node.set_selected(false); }
}

} // namespace pi
