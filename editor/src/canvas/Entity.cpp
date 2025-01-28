
#include "editor/canvas/Entity.hpp"
#include <imgui.h>

namespace pi {

std::unique_ptr<Entity> Entity::clone() const { return std::unique_ptr<Entity>(); }

void Entity::serialize(dj::Json& out) {
	out["id"] = id;
	out["position"][0] = position.x;
	out["position"][1] = position.y;
	out["dimensions"][0] = dimensions.x;
	out["dimensions"][1] = dimensions.y;
}

void Entity::unserialize(dj::Json& in) {
	id = in["id"].as<int>();
	position.x = in["position"][0].as<uint32_t>();
	position.y = in["position"][1].as<uint32_t>();
	dimensions.x = in["dimensions"][0].as<uint32_t>();
	dimensions.y = in["dimensions"][1].as<uint32_t>();
}

void Entity::expose() {
	ImGui::Text("Category: %s", label.c_str());
	ImGui::Separator();
	ImGui::InputInt("Entity ID", &id);
	ImGui::Text("Position: (%i", position.x);
	ImGui::SameLine();
	ImGui::Text(", %i)", position.y);
	ImGui::Separator();
}

void Entity::render(sf::RenderWindow& win, sf::Vector2<float> cam) {}

void Entity::render(sf::RenderWindow& win, sf::Vector2<float> cam, float size) {
	drawbox.setFillColor(sf::Color{255, 60, 60, 80});
	drawbox.setOutlineColor(sf::Color{120, 255, 60, 120});
	drawbox.setOutlineThickness(-2.f);
	if (highlighted) {
		drawbox.setFillColor(sf::Color{255, 100, 60, 180});
		drawbox.setOutlineColor(sf::Color{255, 255, 60, 210});
		drawbox.setOutlineThickness(2.f);
	}
	drawbox.setSize(f_dimensions() * size);
	drawbox.setPosition({position.x * size + cam.x, position.y * size + cam.y});
	win.draw(drawbox);
}

} // namespace pi
