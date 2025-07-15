
#pragma once

#include "editor/canvas/Entity.hpp"

#include <djson/json.hpp>

#include <memory>

namespace pi {

class SavePoint : public Entity {
  public:
	SavePoint(fornani::automa::ServiceProvider& svc, dj::Json const& in);
	SavePoint(fornani::automa::ServiceProvider& svc, int id);
	std::unique_ptr<Entity> clone() const override;
	void serialize(dj::Json& out) override;
	void unserialize(dj::Json const& in) override;
	void expose() override;
	void render(sf::RenderWindow& win, sf::Vector2f cam, float size) override;
};

} // namespace pi
