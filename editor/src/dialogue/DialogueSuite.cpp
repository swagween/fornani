
#include <editor/dialogue/DialogueSuite.hpp>
#include <ranges>

namespace pi {

DialogueSuite::DialogueSuite(sf::Font& font, dj::Json const& in, std::string_view host, std::string_view tag) : m_host{host}, m_tag{tag}, m_font{&font} {
	auto suite_nodes = NodeSet{NodeType::suite};
	for (auto const& set : in[host][tag]["suite"].as_array()) {
		for (auto const& message : set.as_array()) { suite_nodes.push_back(Node(font, message.as_string(), NodeType::suite)); }
		m_suite.push_back(suite_nodes);
		suite_nodes.nodes.clear();
	}
	auto response_nodes = NodeSet{NodeType::response};
	for (auto const& set : in[host][tag]["responses"].as_array()) {
		for (auto const& message : set.as_array()) { response_nodes.push_back(Node(font, message.as_string(), NodeType::response)); }
		m_responses.push_back(response_nodes);
		response_nodes.nodes.clear();
	}
	for (auto const& code : in[host][tag]["codes"].as_array()) {
		auto extras = std::vector<int>{};
		auto source = static_cast<fornani::gui::CodeSource>(code[0].as<int>());
		auto set = code[1].as<int>();
		auto index = code[2].as<int>();
		auto type = static_cast<fornani::gui::MessageCodeType>(code[3].as<int>());
		auto value = code[4].as<int>();
		for (auto const& [i, extra] : std::views::enumerate(code.as_array())) {
			if (i < 5) { continue; }
			extras.push_back(extra.as<int>());
		}
		m_codes.push_back(fornani::gui::MessageCode{source, set, index, type, value, extras});
		auto& src = source == fornani::gui::CodeSource::suite ? m_suite : m_responses;
		src.at(set).nodes.at(index).set_coded(true);
	}
}

void DialogueSuite::serialize(dj::Json& out) {
	out[m_host][m_tag]["suite"] = dj::Json::empty_array();
	for (auto& node_set : m_suite) {
		dj::Json current = dj::Json::empty_array();
		for (auto& node : node_set.nodes) { current.push_back(node.get_message()); }
		out[m_host][m_tag]["suite"].push_back(current);
	}
	out[m_host][m_tag]["responses"] = dj::Json::empty_array();
	for (auto& node_set : m_responses) {
		dj::Json current = dj::Json::empty_array();
		for (auto& node : node_set.nodes) { current.push_back(node.get_message()); }
		out[m_host][m_tag]["responses"].push_back(current);
	}
	out[m_host][m_tag]["codes"] = dj::Json::empty_array();
	for (auto const& code : m_codes) {
		dj::Json current = dj::Json::empty_array();
		current.push_back(static_cast<int>(code.source));
		current.push_back(code.set);
		current.push_back(code.index);
		current.push_back(static_cast<int>(code.type));
		current.push_back(code.value);
		if (code.extras) {
			for (auto const& extra : code.extras.value()) { current.push_back(extra); }
		}
		out[m_host][m_tag]["codes"].push_back(current);
	}
}

void DialogueSuite::add_message(std::string_view message, NodeType type, int set_index, int message_index) {
	auto& source = type == NodeType::suite ? m_suite : m_responses;
	if (source.empty()) {
		source.push_back(NodeSet{type});
		source.back().push_back(Node(*m_font, message.data(), type));
	}
}

void DialogueSuite::add_code(fornani::gui::MessageCodeType type, int value) {
	auto source = static_cast<fornani::gui::CodeSource>(m_current_type);
	auto set = static_cast<int>(m_current_set);
	auto index = static_cast<int>(m_current_index);
	m_codes.push_back(fornani::gui::MessageCode{source, set, index, type, value});
	auto& src = source == fornani::gui::CodeSource::suite ? m_suite : m_responses;
	src.at(set).nodes.at(index).set_coded(true);
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
	std::erase_if(m_codes, [](auto c) { return c.is_marked_for_deletion(); });
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
	for (auto [i, code] : std::views::enumerate(m_codes)) {
		ImGui::PushID(i);
		if (m_current_type == code.source && m_current_set == code.set && m_current_index == code.index) {
			ImGui::Text(">");
			ImGui::SameLine();
		}
		if (ImGui::Button("X##i")) {
			code.mark_for_deletion();
			auto& src = code.source == fornani::gui::CodeSource::suite ? m_suite : m_responses;
			src.at(code.set).nodes.at(code.index).set_coded(false);
		}
		ImGui::SameLine();
		ImGui::Text("[ %i, %i, %i, %i, %i ]", code.source, code.set, code.index, code.type, code.value);
		ImGui::PopID();
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
