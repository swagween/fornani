
#include <fornani/entity/Chest.hpp>
#include <fornani/service/ServiceProvider.hpp>

namespace fornani {

Chest::Chest(automa::ServiceProvider& svc, dj::Json const& in) : Entity(svc, in, "chests"), m_services{&svc} {
	unserialize(in);
	Drawable::set_texture_rect(sf::IntRect{{}, constants::i_resolution_vec});
}

Chest::Chest(automa::ServiceProvider& svc, int type, int modifier, int id) : Entity(svc, "chests", id, {1, 1}), m_type{type}, m_content_modifier{modifier}, m_services{&svc} {
	repeatable = false;
	Drawable::set_texture_rect(sf::IntRect{{}, constants::i_resolution_vec});
}

Chest::Chest(automa::ServiceProvider& svc, int type, std::string tag, int modifier, int id) : Chest(svc, type, modifier, id) {
	m_tag = tag;
	NANI_LOG_INFO(m_logger, "Created a chest with tag [{}]", tag);
}

std::unique_ptr<Entity> Chest::clone() const { return std::make_unique<Chest>(*this); }

void Chest::serialize(dj::Json& out) {
	Entity::serialize(out);
	out["type"] = m_type;
	out["modifier"] = m_content_modifier;
	if (m_tag) { out["tag"] = m_tag.value(); }
}

void Chest::unserialize(dj::Json const& in) {
	Entity::unserialize(in);
	m_type = in["type"].as<int>();
	m_content_modifier = in["modifier"].as<int>();
	if (in["tag"]) { m_tag = in["tag"].as_string(); }
}

void Chest::expose() {
	Entity::expose();
	static int modifier{m_content_modifier};
	static int type{m_type};
	static char const* label{"item_label"};
	static char const* types[3] = {"gun", "orb", "item"};

	if (m_tag.has_value()) { label = m_tag->c_str(); }

	auto ctr{0};
	if (ImGui::BeginCombo("Type", types[type])) {
		for (auto const& t : types) {
			if (ImGui::Selectable(t)) { type = ctr; }
			++ctr;
		}
		ImGui::EndCombo();
	}

	if (type == 0 || type == 2) {
		if (ImGui::BeginCombo("Contents", label, ImGuiComboFlags_HeightLargest)) {
			auto labels = std::vector<std::string>{};
			switch (type) {
			case 0:
				for (auto const& gun : m_services->data.weapon.as_object()) {
					if (ImGui::Selectable(gun.first.c_str())) {
						modifier = gun.second["metadata"]["id"].as<int>();
						label = gun.first.c_str();
						m_tag = std::string{label};
					}
				}
				break;
			case 2:
				for (auto const& item : m_services->data.item.as_array()) { labels.push_back(item["tag"].as_string()); }
				std::ranges::sort(labels, {});
				for (auto const& lbl : labels) {
					if (ImGui::Selectable(lbl.c_str())) {
						label = lbl.c_str();
						m_tag = std::string{label};
					}
				}
				break;
			default: break;
			}
			ImGui::EndCombo();
		}
	} else if (type == 1) {
		ImGui::SliderInt("##rarity", &modifier, 1, 100);
	}
	m_type = type;
	m_content_modifier = modifier;
}

void Chest::render(sf::RenderWindow& win, sf::Vector2f cam, float size) {
	highlighted ? drawbox.setFillColor(sf::Color{250, 120, 80, 80}) : drawbox.setFillColor(sf::Color{250, 120, 80, 10});
	Entity::render(win, cam, size);
}

} // namespace fornani
