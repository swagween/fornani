
#include "editor/canvas/Entity.hpp"
#include <imgui.h>

namespace pi {

Entity::Entity(dj::Json const& in, std::string_view label) : m_label{label} { unserialize(in); }

std::unique_ptr<Entity> Entity::clone() const { return std::unique_ptr<Entity>(); }

void Entity::serialize(dj::Json& out) {
	out["id"] = id;
	out["position"][0] = position.x;
	out["position"][1] = position.y;
	out["dimensions"][0] = dimensions.x;
	out["dimensions"][1] = dimensions.y;
}

void Entity::unserialize(dj::Json const& in) {
	id = in["id"].as<int>();
	NANI_LOG_INFO(m_logger, "Unserializing entity with id {}", id);
	position.x = in["position"][0].as<std::uint32_t>();
	position.y = in["position"][1].as<std::uint32_t>();
	dimensions.x = in["dimensions"][0].as<std::uint32_t>();
	dimensions.y = in["dimensions"][1].as<std::uint32_t>();
}

void Entity::expose() {
	ImGui::Text("Category: %s", m_label.c_str());
	ImGui::Separator();
	ImGui::InputInt("Entity ID", &id);
	ImGui::Text("Position: (%i", position.x);
	ImGui::SameLine();
	ImGui::Text(", %i)", position.y);
	ImGui::Separator();
}

void Entity::set_position(sf::Vector2u to_position) { position = to_position; }

void Entity::render(sf::RenderWindow& win, sf::Vector2f cam, float size) {
	drawbox.setOutlineColor(sf::Color{255, 255, 220, 60});
	drawbox.setOutlineThickness(-2.f);
	if (highlighted) {
		drawbox.setOutlineColor(sf::Color{255, 255, 220, 160});
		drawbox.setOutlineThickness(2.f);
	}
	drawbox.setSize(f_dimensions() * size);
	drawbox.setPosition({position.x * size + cam.x, position.y * size + cam.y});
	win.draw(drawbox);
}

} // namespace pi
