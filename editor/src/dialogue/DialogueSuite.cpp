
#include <editor/dialogue/DialogueSuite.hpp>
#include <ranges>

namespace pi {

DialogueSuite::DialogueSuite(sf::Font& font, dj::Json const& in, std::string_view host, std::string_view tag) : m_host{host}, m_tag{tag}, m_font{&font} {
	auto suite_nodes = NodeSet{NodeType::suite};
	for (auto const& set : in[host][tag]["suite"].as_array()) {
		for (auto const& message : set.as_array()) { suite_nodes.push_back(Node(message["codes"], font, message["message"].as_string(), NodeType::suite)); }
		m_suite.push_back(suite_nodes);
		suite_nodes.nodes.clear();
	}
	auto response_nodes = NodeSet{NodeType::response};
	for (auto const& set : in[host][tag]["responses"].as_array()) {
		for (auto const& message : set.as_array()) { response_nodes.push_back(Node(message["codes"], font, message["message"].as_string(), NodeType::response)); }
		m_responses.push_back(response_nodes);
		response_nodes.nodes.clear();
	}
}

void DialogueSuite::serialize(dj::Json& out) {
	out[m_host][m_tag]["suite"] = dj::Json::empty_array();
	for (auto& node_set : m_suite) {
		dj::Json current = dj::Json::empty_array();
		for (auto& node : node_set.nodes) {
			auto message = dj::Json::empty_object();
			node.serialize(message);
			current.push_back(message);
		}
		out[m_host][m_tag]["suite"].push_back(current);
	}
	out[m_host][m_tag]["responses"] = dj::Json::empty_array();
	for (auto& node_set : m_responses) {
		dj::Json current = dj::Json::empty_array();
		for (auto& node : node_set.nodes) {
			auto message = dj::Json::empty_object();
			node.serialize(message);
			current.push_back(message);
		}
		out[m_host][m_tag]["responses"].push_back(current);
	}
}

void DialogueSuite::add_set(std::string_view message, NodeType type) {
	auto& source = type == NodeType::suite ? m_suite : m_responses;
	source.push_back(NodeSet{type});
	source.back().push_back(Node(*m_font, message.data(), type));
}

void DialogueSuite::add_message(std::string_view message, NodeType type) {
	auto& source = type == NodeType::suite ? m_suite : m_responses;
	if (source.empty()) { source.push_back(NodeSet{type}); }
	source.back().push_back(Node(*m_font, message.data(), type));
}

void DialogueSuite::add_code(fornani::gui::MessageCodeType type, int value) {
	auto source = static_cast<fornani::gui::CodeSource>(m_current_type);
	auto set = static_cast<int>(m_current_set);
	auto index = static_cast<int>(m_current_index);
	auto& src = source == fornani::gui::CodeSource::suite ? m_suite : m_responses;
	src.at(set).nodes.at(index).add_code({static_cast<int>(type), value});
}

void DialogueSuite::update(sf::Vector2f position, bool clicked) {
	auto previous_position = sf::Vector2f{};
	auto buffer = 8.f;
	for (auto const i : {0, 1}) {
		auto& source = i == 0 ? m_suite : m_responses;
		for (auto [n, node_set] : std::views::enumerate(source)) {
			for (auto [i, node] : std::views::enumerate(node_set.nodes)) {
				node.update(position, clicked);
				if (node.is_selected()) {
					m_current_type = node.get_type();
					m_current_index = i;
					m_current_set = n;
				}
			}
			node_set.update(position);
			node_set.set_position(previous_position);
			previous_position.y += node_set.get_size().y + buffer;
		}
	}
}

void DialogueSuite::render(sf::RenderWindow& win, sf::Vector2f cam) {
	for (auto& node_set : m_suite) { node_set.render(win, cam); }
	for (auto& node_set : m_responses) { node_set.render(win, cam); }
}

void DialogueSuite::swap_node(Node other) {
	auto& source = m_current_type == NodeType::suite ? m_suite : m_responses;
	if (m_current_set >= source.size()) { return; }
	if (m_current_index >= source.at(m_current_set).nodes.size()) { return; }
	source.at(m_current_set).nodes.at(m_current_index) = other;
}

void DialogueSuite::print_codes() {
	for (auto& node_set : m_suite) {
		for (auto& node : node_set.nodes) {
			if (node.is_selected()) {
				ImGui::Text("Suite Codes");
				node.print_codes();
			}
		}
	}
	for (auto& node_set : m_responses) {
		for (auto& node : node_set.nodes) {
			if (node.is_selected()) {
				ImGui::Text("Response Codes");
				node.print_codes();
			}
		}
	}
}

auto DialogueSuite::get_current_node() const -> std::optional<Node> {
	auto& source = m_current_type == NodeType::suite ? m_suite : m_responses;
	if (m_current_set >= source.size()) { return std::nullopt; }
	if (m_current_index >= source.at(m_current_set).nodes.size()) { return std::nullopt; }
	return source.at(m_current_set).nodes.at(m_current_index);
}

auto DialogueSuite::is_any_node_hovered() const -> bool {
	for (auto& node_set : m_suite) {
		for (auto& node : node_set.nodes) {
			if (node.is_hovered()) { return true; }
		}
	}
	for (auto& node_set : m_responses) {
		for (auto& node : node_set.nodes) {
			if (node.is_hovered()) { return true; }
		}
	}
	return false;
}

auto DialogueSuite::is_any_node_selected() const -> bool {
	for (auto& node_set : m_suite) {
		for (auto& node : node_set.nodes) {
			if (node.is_selected()) { return true; }
		}
	}
	for (auto& node_set : m_responses) {
		for (auto& node : node_set.nodes) {
			if (node.is_selected()) { return true; }
		}
	}
	return false;
}

void DialogueSuite::unhover_all() {
	for (auto& node_set : m_suite) { node_set.unhover_all(); }
	for (auto& node_set : m_responses) { node_set.unhover_all(); }
}

void DialogueSuite::deselect_all() {
	for (auto& node_set : m_suite) { node_set.deselect_all(); }
	for (auto& node_set : m_responses) { node_set.deselect_all(); }
}

} // namespace pi
