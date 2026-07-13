
#include <imgui.h>
#include <fornani/core/Debug.hpp>
#include <fornani/entity/Entity.hpp>
#include <fornani/graphics/Colors.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/utils/Constants.hpp>

namespace fornani {

Entity::Entity(automa::ServiceProvider& svc, dj::Json const& in, std::string_view label, sf::Vector2i dim)
	: p_animatable(svc, label, dim), m_label{label}, IWorldPositionable({in["position"][0].as<std::uint32_t>(), in["position"][1].as<std::uint32_t>()}, {in["dimensions"][0].as<std::uint32_t>(), in["dimensions"][1].as<std::uint32_t>()}) {
	unserialize(in);
	m_editor = svc.is_editor();
	if (p_contingencies) {
		if (!svc.quest_table.are_contingencies_met(*p_contingencies)) { p_flags.set(EntityFlags::spawn_denied); }
	}
}

Entity::Entity(automa::ServiceProvider& svc, std::string_view label, int to_id, sf::Vector2<std::uint32_t> dim) : p_animatable(svc, label), m_id{to_id}, m_label{label}, IWorldPositionable{{}, dim} { m_editor = svc.is_editor(); }

std::unique_ptr<Entity> Entity::clone() const { return std::unique_ptr<Entity>(); }

void Entity::serialize(dj::Json& out) {
	out["id"] = m_id;
	out["position"][0] = get_grid_position().x;
	out["position"][1] = get_grid_position().y;
	out["dimensions"][0] = get_grid_dimensions().x;
	out["dimensions"][1] = get_grid_dimensions().y;
	if (p_contingencies) { p_contingencies->serialize(out["contingencies"]); }
}

void Entity::unserialize(dj::Json const& in) {
	m_id = in["id"].as<int>();
	if (in["contingencies"]) { p_contingencies.emplace(in["contingencies"]); }
}

void Entity::expose() {
	static int w = IWorldPositionable::m_dimensions.x;
	static int h = IWorldPositionable::m_dimensions.y;
	ImGui::Text("Category: %s", m_label.c_str());
	ImGui::Separator();
	ImGui::InputInt("Entity ID", &m_id);
	ImGui::Text("Position: (%i", get_grid_position().x);
	ImGui::SameLine();
	ImGui::Text(", %i)", get_grid_position().y);
	ImGui::Separator();
	ImGui::InputInt("Width", &w);
	ImGui::InputInt("Height", &h);
	set_grid_dimensions(sf::Vector2i{w, h});
	ImGui::Text("Quest Contingencies");
	static char tag_buffer[256] = "";
	static int requirement{};
	static bool strict{};
	ImGui::InputTextWithHint("Tag", "Quest Tag", tag_buffer, IM_ARRAYSIZE(tag_buffer));
	ImGui::InputInt("Requirement", &requirement);
	ImGui::Checkbox("Strict?", &strict);
	if (ImGui::Button("Add Contingency")) {
		auto ct = QuestContingency{tag_buffer, requirement, strict};
		if (!p_contingencies) {
			p_contingencies.emplace({ct});
		} else {
			p_contingencies->add(ct);
		}
	}
	ImGui::NewLine();
	ImGui::Separator();
	ImGui::Text("Current List:");
	if (p_contingencies) {
		for (auto [i, ct] : std::views::enumerate(p_contingencies->contingencies)) {
			ImGui::PushID(i);
			if (ImGui::SmallButton("x")) { ct.delete_me = true; }
			ImGui::SameLine();
			ImGui::Text("[%s, %i]", ct.tag.c_str(), ct.requirement);
			ImGui::PopID();
		}
		std::erase_if(p_contingencies->contingencies, [](auto const& ct) { return ct.delete_me; });
	} else {
		ImGui::Text("<none>");
	}
}

void Entity::set_position(sf::Vector2u to_position) { set_grid_position(to_position); }

auto Entity::contains_point(sf::Vector2u test) const -> bool {
	for (auto i{0u}; i < get_grid_dimensions().x; ++i) {
		for (auto j{0u}; j < get_grid_dimensions().y; ++j) {
			if (get_grid_position() + sf::Vector2u{i, j} == test) { return true; }
		}
	}
	return false;
}

void Entity::update([[maybe_unused]] automa::ServiceProvider& svc, [[maybe_unused]] world::Map& map, [[maybe_unused]] SceneContext& context, [[maybe_unused]] player::Player& player) { p_animatable.tick(); }

void Entity::render(sf::RenderWindow& win, sf::Vector2f cam, float size) {
	if (!m_editor) { return; }
	drawbox.setOutlineColor(colors::blue);
	drawbox.setOutlineThickness(-2.f);
	if (highlighted) {
		drawbox.setOutlineColor(colors::ui_white);
		drawbox.setOutlineThickness(2.f);
	}
	drawbox.setSize(get_f_grid_dimensions() * size);
	drawbox.setPosition(get_f_grid_position() * size + cam);
	p_animatable.set_scale(constants::f_scale_vec * size / constants::f_cell_size);
	p_animatable.set_position(get_f_grid_position() * size + cam);
	if (m_textured) { win.draw(p_animatable); }
	win.draw(drawbox);
}

void Entity::submit(Renderer& renderer) {}

} // namespace fornani
