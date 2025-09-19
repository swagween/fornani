
#include <imgui.h>
#include <editor/dialogue/Node.hpp>
#include <ranges>

namespace pi {

constexpr auto box_buffer = sf::Vector2f{20.f, 20.f};

Node::Node(dj::Json const& in_code, sf::Font const& font, std::string_view message, NodeType type) : Node(font, message, type) {
	for (auto const& code : in_code.as_array()) {
		m_codes.push_back(fornani::gui::MessageCode{code});
		m_coded = true;
	}
}

Node::Node(sf::Font const& font, std::string_view message, NodeType type) : m_text{sf::Text{font}}, m_type{type} {
	if (m_text) {
		m_text->setString(message.data());
		type == NodeType::suite ? m_text->setFillColor(fornani::colors::pioneer_red) : m_text->setFillColor(fornani::colors::dark_goldenrod);
		m_text->setCharacterSize(16);
		box.setSize(m_text->getLocalBounds().size + box_buffer);
		m_text->setOrigin(m_text->getLocalBounds().getCenter());
	}
	type == NodeType::suite ? box.setOutlineColor(fornani::colors::pioneer_red) : box.setOutlineColor(fornani::colors::dark_goldenrod);
	m_code_indicator.setSize({8.f, 8.f});
	m_code_indicator.setOrigin(sf::Vector2f{box_buffer.x * 2.f, -box.getSize().y * 0.5f} + m_code_indicator.getLocalBounds().getCenter());
}

Node& Node::operator=(Node const& other) {
	if (this != &other) {
		m_text = other.m_text;
		m_position = other.m_position;
		box = other.box;
		m_type = other.m_type;
		m_hovered = other.m_hovered;
	}
	return *this;
}

void Node::serialize(dj::Json& out) {
	out["message"] = get_message();
	for (auto& code : m_codes) { code.serialize(out["codes"]); }
}

void Node::add_code(std::vector<int> code) {
	m_codes.push_back(fornani::gui::MessageCode{code});
	m_coded = true;
}

void Node::update(sf::Vector2f position, bool clicked) {
	m_hovered = box.getGlobalBounds().contains(position);
	if (m_hovered && clicked) { m_selected = true; }
	std::erase_if(m_codes, [](auto const& c) { return c.is_marked_for_deletion(); });
	if (m_codes.empty()) { m_coded = false; }
}

void Node::render(sf::RenderWindow& win, sf::Vector2f cam) {
	box.setPosition(m_position - cam);
	m_code_indicator.setPosition(m_position - cam);
	m_hovered || m_selected ? box.setFillColor(fornani::colors::pioneer_dark_red) : box.setFillColor(sf::Color::Transparent);
	m_hovered || m_selected ? box.setOutlineThickness(-4.f) : box.setOutlineThickness(-2.f);
	m_hovered || m_selected ? m_code_indicator.setFillColor(fornani::colors::periwinkle) : m_code_indicator.setFillColor(fornani::colors::dark_grey);
	m_type == NodeType::suite ? box.setOutlineColor(fornani::colors::pioneer_red) : box.setOutlineColor(fornani::colors::dark_goldenrod);
	if (m_selected) { box.setOutlineColor(fornani::colors::ui_white); }
	win.draw(box);
	if (m_text) {
		m_text->setPosition(box.getGlobalBounds().getCenter());
		win.draw(*m_text);
	}
	if (m_coded) { win.draw(m_code_indicator); }
}

void Node::print_codes() {
	for (auto [i, code] : std::views::enumerate(m_codes)) {
		ImGui::PushID(i);
		if (ImGui::SmallButton("X")) { code.delete_me = true; }
		ImGui::SameLine();
		ImGui::Text("[%i, %i]", static_cast<int>(code.type), code.value);
		ImGui::PopID();
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
