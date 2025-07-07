
#include "editor/canvas/entity/Inspectable.hpp"

namespace pi {

Inspectable::Inspectable() : Entity("inspectables") {}

Inspectable::Inspectable(bool activate_on_contact, std::string key, std::vector<std::vector<std::string>> suites, std::vector<std::vector<std::string>> responses, int alternates)
	: Entity("inspectables", 0, {1, 1}), activate_on_contact(activate_on_contact), key(key), suites(suites), responses(responses), alternates(alternates) {}

std::unique_ptr<Entity> Inspectable::clone() const { return std::make_unique<Inspectable>(*this); }

void Inspectable::serialize(dj::Json& out) {
	Entity::serialize(out);
	auto const& wipe = dj::Json::empty_array();
	out["activate_on_contact"] = activate_on_contact;
	out["key"] = key;
	out["alternates"] = alternates;
	for (auto i{0}; i <= alternates; ++i) {
		auto next = std::string{key + std::to_string(i)};
		out[next].push_back(wipe);
		for (auto& suite : suites) {
			auto out_set = wipe;
			for (auto& message : suite) { out_set.push_back(message); }
			out[next]["suite"].push_back(out_set);
		}
		for (auto& response : responses) {
			auto out_set = wipe;
			for (auto& message : response) { out_set.push_back(message); }
			out[next]["responses"].push_back(out_set);
		}
	}
}

void Inspectable::unserialize(dj::Json const& in) {
	Entity::unserialize(in);
	auto const& wipe = dj::Json::empty_array();
	activate_on_contact = static_cast<bool>(in["activate_on_contact"].as_bool());
	key = in["key"].as_string();
	alternates = in["alternates"].as<int>();
	for (auto i{0}; i <= alternates; ++i) {
		auto next = std::string{key + std::to_string(i)};
		for (auto& in_suite : in[next]["suite"].as_array()) {
			for (auto& message : in_suite.as_array()) { suites.push_back(std::vector<std::string>{message.as_string().data()}); }
		}
		for (auto& in_response : in[next]["responses"].as_array()) {
			for (auto& message : in_response.as_array()) { responses.push_back(std::vector<std::string>{message.as_string().data()}); }
		}
	}
}

void Inspectable::expose() {
	Entity::expose();
	ImGui::Checkbox("Activate on Contact", &activate_on_contact);
	ImGui::InputInt("Alternates", &alternates);
}

void Inspectable::render(sf::RenderWindow& win, sf::Vector2f cam, float size) {
	highlighted ? drawbox.setFillColor(sf::Color{255, 100, 60, 180}) : drawbox.setFillColor(sf::Color{255, 60, 60, 80});
	Entity::render(win, cam, size);
}

} // namespace pi
