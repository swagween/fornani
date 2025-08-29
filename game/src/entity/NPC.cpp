
#include <fornani/entity/NPC.hpp>
#include <fornani/service/ServiceProvider.hpp>

namespace fornani {

NPC::NPC(automa::ServiceProvider& svc, dj::Json const& in) : Entity(svc, in, "npcs") {
	unserialize(in);
	repeatable = false;
	auto tag = svc.data.get_npc_label_from_id(get_id());
	if (tag) {
		set_texture(svc.assets.get_texture("npc_" + std::string{tag.value()}));
		center();
	}
}

NPC::NPC(automa::ServiceProvider& svc, int id, std::string_view label, std::vector<std::vector<int>> const suites) : Entity(svc, "npcs", id, {1, 1}), m_label{label}, m_suites{suites} {
	repeatable = false;
	auto tag = svc.data.get_npc_label_from_id(get_id());
	if (tag) {
		set_texture(svc.assets.get_texture("npc_" + std::string{tag.value()}));
		center();
	}
}

std::unique_ptr<Entity> NPC::clone() const { return std::make_unique<NPC>(*this); }

void NPC::serialize(dj::Json& out) {
	Entity::serialize(out);
	out["label"] = m_label;
	for (auto& suite : m_suites) {
		auto entry = dj::Json::empty_array();
		for (auto& set : suite) { entry.push_back(set); }
		out["suites"].push_back(entry);
	}
}

void NPC::unserialize(dj::Json const& in) {
	Entity::unserialize(in);
	m_label = in["label"].as_string();
	for (auto const& suite : in["suites"].as_array()) {
		auto entry = std::vector<int>{};
		for (auto const& set : suite.as_array()) { entry.push_back(set.as<int>()); }
		m_suites.push_back(entry);
	}
}

void NPC::expose() { Entity::expose(); }

void NPC::render(sf::RenderWindow& win, sf::Vector2f cam, float size) {
	highlighted ? drawbox.setFillColor(sf::Color{250, 80, 250, 60}) : drawbox.setFillColor(sf::Color::Transparent);
	Entity::render(win, cam, size);
}

} // namespace fornani
