
#include "editor/canvas/entity/Inspectable.hpp"

namespace pi{

	Inspectable::Inspectable(std::string label) : Entity("inspectables") {}
Inspectable::Inspectable(bool activate_on_contact, std::string key, std::vector<std::vector<std::string>> suites, std::vector<std::vector<std::string>> responses, int alternates)
		: Entity("inspectables", 0, {}, {1, 1}), activate_on_contact(activate_on_contact), key(key), suites(suites), responses(responses), alternates(alternates) {}

	std::unique_ptr<Entity> Inspectable::clone() const override { return std::make_unique<Inspectable>(*this); }
void Inspectable::serialize(dj::Json& out) override {
		Entity::serialize(out);
		constexpr auto empty_array = R"([])";
		auto const wipe = dj::Json::parse(empty_array);
		out["activate_on_contact"] = dj::Boolean{activate_on_contact};
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
void Inspectable::unserialize(dj::Json& in) override {
		Entity::unserialize(in);
		constexpr auto empty_array = R"([])";
		auto const wipe = dj::Json::parse(empty_array);
		activate_on_contact = static_cast<bool>(in["activate_on_contact"].as_bool());
		key = in["key"].as_string();
		alternates = in["alternates"].as<int>();
		for (auto i{0}; i <= alternates; ++i) {
			auto next = std::string{key + std::to_string(i)};
			for (auto& in_suite : in[next]["suite"].array_view()) {
				for (auto& message : in_suite.array_view()) { suites.push_back(std::vector<std::string>{message.as_string().data()}); }
			}
			for (auto& in_response : in[next]["responses"].array_view()) {
				for (auto& message : in_response.array_view()) { responses.push_back(std::vector<std::string>{message.as_string().data()}); }
			}
		}
	}
void Inspectable::expose() override {
		Entity::expose();
		ImGui::Checkbox("Activate on Contact", &activate_on_contact);
		ImGui::InputInt("Alternates", &alternates);
	}
void Inspectable::render(sf::RenderWindow& win, sf::Vector2<float> cam) override { Entity::render(win, cam); };
}
