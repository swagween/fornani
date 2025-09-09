
#include <fornani/entity/Inspectable.hpp>
#include <fornani/service/ServiceProvider.hpp>

namespace fornani {

Inspectable::Inspectable(automa::ServiceProvider& svc, dj::Json const& in) : Entity(svc, in, "inspectables") {
	unserialize(in);
	m_textured = false;
}

Inspectable::Inspectable(automa::ServiceProvider& svc, std::vector<std::vector<gui::BasicMessage>> suite, std::vector<std::vector<gui::BasicMessage>> responses, bool activate_on_contact, std::string key, int alternates, bool instant)
	: Entity(svc, "inspectables", 0, {1, 1}), m_activate_on_contact{activate_on_contact}, m_key{key}, m_alternates{alternates}, m_instant{instant}, m_suites{suite}, m_responses{responses} {
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
	out["series"] = dj::Json::empty_array();
	for (auto i{0}; i < m_suites.size(); ++i) {
		auto next = dj::Json{};
		for (auto j = 0; j < 2; ++j) {
			auto& from_set = j == 0 ? m_suites : m_responses;
			auto tag = j == 0 ? "suite" : "responses";
			for (auto& suite : from_set) {
				auto out_set = dj::Json{};
				for (auto const& message : suite) {
					auto msg = dj::Json{};
					if (message.codes) {
						for (auto const& code : message.codes.value()) { code.serialize(msg["codes"]); }
					}
					msg["message"] = message.data;
					out_set.push_back(msg);
				}
				next[tag].push_back(out_set);
			}
		}
		out["series"].push_back(next);
	}
}

void Inspectable::unserialize(dj::Json const& in) {
	Entity::unserialize(in);
	auto const& wipe = dj::Json::empty_array();
	m_activate_on_contact = in["activate_on_contact"].as_bool();
	m_instant = in["instant"].as_bool();
	m_key = in["key"].as_string();
	m_alternates = in["alternates"].as<int>();
	for (auto const& entry : in["series"].as_array()) {
		for (auto j = 0; j < 2; ++j) {
			auto& to_set = j == 0 ? m_suites : m_responses;
			auto tag = j == 0 ? "suite" : "responses";
			for (auto const& in_suite : entry[tag].as_array()) {
				auto s = std::vector<gui::BasicMessage>{};
				for (auto const& message : in_suite.as_array()) {
					auto codes = std::vector<gui::MessageCode>{};
					for (auto const& c : message["codes"].as_array()) {
						auto this_code = gui::MessageCode{c};
						codes.push_back(this_code);
					}
					s.push_back(gui::BasicMessage{message["message"].as_string(), codes});
				}
				to_set.push_back(s);
			}
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
