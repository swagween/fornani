
#pragma once

#include "editor/canvas/Entity.hpp"

#include <djson/json.hpp>

#include <memory>

namespace pi {

class Enemy : public Entity {
  public:
	Enemy(fornani::automa::ServiceProvider& svc, dj::Json const& in);
	Enemy(fornani::automa::ServiceProvider& svc, int id, int variant = 0);
	std::unique_ptr<Entity> clone() const override;
	void serialize(dj::Json& out) override;
	void unserialize(dj::Json const& in) override;
	void expose() override;
	void render(sf::RenderWindow& win, sf::Vector2f cam, float size) override;

  private:
	int m_variant{};
};

} // namespace pi
