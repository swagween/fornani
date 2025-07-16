
#pragma once

#include "editor/canvas/Entity.hpp"

namespace pi {

class SwitchBlock : public Entity {
  public:
	SwitchBlock(fornani::automa::ServiceProvider& svc, dj::Json const& in);
	SwitchBlock(fornani::automa::ServiceProvider& svc, int id, int type);
	std::unique_ptr<Entity> clone() const override;
	void serialize(dj::Json& out) override;
	void unserialize(dj::Json const& in) override;
	void expose() override;
	void render(sf::RenderWindow& win, sf::Vector2f cam, float size) override;

  private:
	int m_type{};
};

} // namespace pi
