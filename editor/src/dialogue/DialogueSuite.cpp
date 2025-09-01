
#include <editor/dialogue/DialogueSuite.hpp>

namespace pi {

DialogueSuite::DialogueSuite(sf::Font const& font, dj::Json const& in, std::string_view host, std::string_view tag) : m_host{host}, m_tag{tag} {
	auto suite_set = std::vector<Node>{};
	for (auto const& set : in[host][tag]["suite"].as_array()) {
		for (auto const& message : set.as_array()) { suite_set.push_back(Node(font, message.as_string())); }
		m_nodes.push_back(suite_set);
	}
}

void DialogueSuite::serialize(dj::Json& out) {
	for (auto& set : m_suite) {
		dj::Json current = dj::Json::empty_array();
		for (auto& message : set) { current.push_back(message); }
		out[m_host][m_tag]["suite"].push_back(current);
	}
}

void DialogueSuite::add_message(std::string_view message, int set_index, int message_index) {
	if (m_suite.empty()) { m_suite.push_back(std::vector<std::string>{}); }
	m_suite.back().push_back(message.data());
}

void DialogueSuite::render(sf::RenderWindow& win, sf::Vector2f cam) {
	sf::Vector2f previous_position{};
	for (auto& node_set : m_nodes) {
		for (auto& node : node_set) {
			node.set_position(previous_position + sf::Vector2f{0.f, node.get_size().y});
			previous_position = node.get_position();
			node.render(win, cam);
		}
	}
}

} // namespace pi
