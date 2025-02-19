
#pragma once

#include "editor/canvas/Entity.hpp"

namespace pi {

class Portal : public Entity {
  public:
	Portal(sf::Vector2u dimensions, bool activate_on_contact, bool already_open, int source_map_id, int destination_map_id, bool locked, int key_id);

	std::unique_ptr<Entity> clone() const override;
	void serialize(dj::Json& out) override;
	void unserialize(dj::Json& in) override;
	void expose() override;
	void render(sf::RenderWindow& win, sf::Vector2<float> cam);

  private:
	bool activate_on_contact{};
	bool already_open{};
	int source_map_id{};
	int destination_map_id{};
	bool locked{};
	int key_id{};
};

} // namespace pi
