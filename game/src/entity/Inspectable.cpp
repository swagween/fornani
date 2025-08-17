
#include <fornani/entity/Inspectable.hpp>
#include <fornani/service/ServiceProvider.hpp>

namespace fornani {

Inspectable::Inspectable(automa::ServiceProvider& svc, dj::Json const& in) : Entity(svc, in, "inspectables") {
	unserialize(in);
	m_textured = false;
}

Inspectable::Inspectable(automa::ServiceProvider& svc, bool activate_on_contact, std::string key, std::vector<std::vector<std::string>> suites, std::vector<std::vector<std::string>> responses, int alternates, bool instant)
	: Entity(svc, "inspectables", 0, {1, 1}), m_activate_on_contact{activate_on_contact}, m_key{key}, m_suites{suites}, m_responses{responses}, m_alternates{alternates}, m_instant{instant} {
	m_textured = false;
}

std::unique_ptr<Entity> Inspectable::clone() const { return std::make_unique<Inspectable>(*this); }

void Inspectable::serialize(dj::Json& out) {
	Entity::serialize(out);
	auto const& wipe = dj::Json::empty_array();
	out["activate_on_contact"] = m_activate_on_contact;
	out["instant"] = m_instant;
	out["key"] = m_key;
	out["alternates"] = m_alternates;
	for (auto i{0}; i <= m_alternates; ++i) {
		auto next = std::string{m_key + std::to_string(i)};
		out[next].push_back(wipe);
		for (auto& suite : m_suites) {
			auto out_set = wipe;
			for (auto const& message : suite) { out_set.push_back(message); }
			out[next]["suite"].push_back(out_set);
		}
		for (auto& response : m_responses) {
			auto out_set = wipe;
			for (auto const& message : response) { out_set.push_back(message); }
			out[next]["responses"].push_back(out_set);
		}
		for (auto const& code_set : m_codes) {
			auto out_code = wipe;
			for (auto const& code : code_set) { out_code.push_back(code); }
			out[next]["codes"].push_back(out_code);
		}
	}
}

void Inspectable::unserialize(dj::Json const& in) {
	Entity::unserialize(in);
	auto const& wipe = dj::Json::empty_array();
	m_activate_on_contact = in["activate_on_contact"].as_bool();
	m_instant = in["instant"].as_bool();
	m_key = in["key"].as_string();
	m_alternates = in["alternates"].as<int>();
	for (auto i{0}; i <= m_alternates; ++i) {
		auto next = std::string{m_key + std::to_string(i)};
		for (auto const& in_suite : in[next]["suite"].as_array()) {
			auto s = std::vector<std::string>{};
			for (auto const& message : in_suite.as_array()) { s.push_back(message.as_string().data()); }
			m_suites.push_back(s);
		}
		for (auto const& in_response : in[next]["responses"].as_array()) {
			auto r = std::vector<std::string>{};
			for (auto const& message : in_response.as_array()) { r.push_back(message.as_string().data()); }
			m_responses.push_back(r);
		}
		for (auto const& in_codes : in[next]["codes"].as_array()) {
			auto c = std::vector<int>{};
			for (auto const& code : in_codes.as_array()) { c.push_back(code.as<int>()); }
			m_codes.push_back(c);
		}
	}
}

void Inspectable::expose() {
	Entity::expose();
	ImGui::Checkbox("Activate on Contact", &m_activate_on_contact);
	ImGui::Checkbox("Instant", &m_instant);
	ImGui::InputInt("Alternates", &m_alternates);
}

void Inspectable::render(sf::RenderWindow& win, sf::Vector2f cam, float size) {
	highlighted ? drawbox.setFillColor(sf::Color{255, 100, 60, 180}) : drawbox.setFillColor(sf::Color{255, 60, 60, 80});
	Entity::render(win, cam, size);
}

} // namespace fornani
