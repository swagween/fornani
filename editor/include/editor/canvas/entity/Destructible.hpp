
#pragma once

#include "editor/canvas/Entity.hpp"

namespace pi {

class Destructible : public Entity {
  public:
	Destructible(dj::Json const& in);
	std::unique_ptr<Entity> clone() const override;
	void serialize(dj::Json& out) override;
	void unserialize(dj::Json const& in) override;
	void expose() override;
	void render(sf::RenderWindow& win, sf::Vector2f cam, float size) override;
};

} // namespace pi
