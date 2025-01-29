
#include "editor/canvas/entity/Portal.hpp"

namespace pi {

Portal::Portal(sf::Vector2u dimensions, bool activate_on_contact, bool already_open, int source_map_id, int destination_map_id, bool locked, int key_id)
	: Entity("portals", 0, {}, dimensions), activate_on_contact(activate_on_contact), already_open(already_open), source_map_id(source_map_id), destination_map_id(destination_map_id), locked(locked), key_id(key_id) {}

std::unique_ptr<Entity> Portal::clone() const { return std::make_unique<Portal>(*this); }

void Portal::serialize(dj::Json& out) {
	Entity::serialize(out);
	out["activate_on_contact"] = dj::Boolean{activate_on_contact};
	out["already_open"] = dj::Boolean{already_open};
	out["source_map_id"] = source_map_id;
	out["destination_map_id"] = destination_map_id;
	out["locked"] = dj::Boolean{locked};
	out["key_id"] = key_id;
}

void Portal::unserialize(dj::Json& in) {
	Entity::unserialize(in);
	activate_on_contact = static_cast<bool>(in["activate_on_contact"].as_bool());
	already_open = static_cast<bool>(in["already_open"].as_bool());
	source_map_id = in["source_map_id"].as<int>();
	destination_map_id = in["destination_map_id"].as<int>();
	locked = static_cast<bool>(in["locked"].as_bool());
	key_id = in["key_id"].as<int>();
}

void Portal::expose() { Entity::expose(); }

void Portal::render(sf::RenderWindow& win, sf::Vector2<float> cam) {}

} // namespace pi
