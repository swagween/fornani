
#include "editor/canvas/entity/Portal.hpp"

namespace pi {

Portal::Portal(sf::Vector2u dimensions, bool activate_on_contact, bool already_open, int source_id, int destination_id, bool locked, int key_id)
	: Entity("portals", 0, dimensions), activate_on_contact(activate_on_contact), already_open(already_open), source_id(source_id), destination_id(destination_id), locked(locked), key_id(key_id) {}

Portal::Portal(dj::Json const& in) : Entity(in, "portals") { unserialize(in); }

std::unique_ptr<Entity> Portal::clone() const { return std::make_unique<Portal>(*this); }

void Portal::serialize(dj::Json& out) {
	Entity::serialize(out);
	out["activate_on_contact"] = activate_on_contact;
	out["already_open"] = already_open;
	out["source_id"] = source_id;
	out["destination_id"] = destination_id;
	out["locked"] = locked;
	out["key_id"] = key_id;
}

void Portal::unserialize(dj::Json const& in) {
	activate_on_contact = in["activate_on_contact"].as_bool();
	already_open = in["already_open"].as_bool();
	source_id = in["source_id"].as<int>();
	destination_id = in["destination_id"].as<int>();
	locked = in["locked"].as_bool();
	key_id = in["key_id"].as<int>();
}

void Portal::expose() {
	Entity::expose();
	ImGui::InputInt("Source Room ID", &source_id);
	ImGui::InputInt("Destination Room ID", &destination_id);
	ImGui::Separator();
	ImGui::Checkbox("Activate on Contact", &activate_on_contact);
	ImGui::Checkbox("Already Open", &already_open);
	ImGui::Separator();
	ImGui::Checkbox("Locked", &locked);
	ImGui::InputInt("Key ID", &key_id);
}

void Portal::render(sf::RenderWindow& win, sf::Vector2f cam, float size) {
	highlighted ? drawbox.setFillColor(sf::Color{60, 255, 120, 180}) : drawbox.setFillColor(sf::Color{60, 255, 120, 80});
	Entity::render(win, cam, size);
}

} // namespace pi
