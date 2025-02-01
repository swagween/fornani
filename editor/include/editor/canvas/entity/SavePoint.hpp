
#pragma once

#include "editor/canvas/Entity.hpp"

#include <djson/json.hpp>

#include <memory>

namespace pi {

class SavePoint : public Entity {
  public:
	SavePoint(int id);
	std::unique_ptr<Entity> clone() const override;
	void serialize(dj::Json& out) override;
	void unserialize(dj::Json& in) override;
	void expose() override;
	void render(sf::RenderWindow& win, sf::Vector2<float> cam, float size) override;
};

} // namespace pi
