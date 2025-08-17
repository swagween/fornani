
#pragma once

#include <fornani/entity/Entity.hpp>

namespace fornani {

class SwitchBlock : public Entity {
  public:
	SwitchBlock(automa::ServiceProvider& svc, dj::Json const& in);
	SwitchBlock(automa::ServiceProvider& svc, int id, int type);
	std::unique_ptr<Entity> clone() const override;
	void serialize(dj::Json& out) override;
	void unserialize(dj::Json const& in) override;
	void expose() override;
	void render(sf::RenderWindow& win, sf::Vector2f cam, float size) override;

  private:
	int m_type{};
};

} // namespace fornani
