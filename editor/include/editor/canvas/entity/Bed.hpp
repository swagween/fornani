
#pragma once

#include "editor/canvas/Entity.hpp"

namespace pi {

class Bed : public Entity {
  public:
	Bed(fornani::automa::ServiceProvider& svc, dj::Json const& in);
	Bed(fornani::automa::ServiceProvider& svc, int id, bool flipped);
	std::unique_ptr<Entity> clone() const override;
	void serialize(dj::Json& out) override;
	void unserialize(dj::Json const& in) override;
	void expose() override;
	void render(sf::RenderWindow& win, sf::Vector2f cam, float size) override;

  private:
	bool m_flipped{};
};

} // namespace pi
