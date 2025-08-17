
#pragma once

#include <fornani/entity/Entity.hpp>

namespace fornani {

class SavePoint : public Entity {
  public:
	SavePoint(automa::ServiceProvider& svc, dj::Json const& in);
	SavePoint(automa::ServiceProvider& svc, int id);
	std::unique_ptr<Entity> clone() const override;
	void serialize(dj::Json& out) override;
	void unserialize(dj::Json const& in) override;
	void expose() override;
	void render(sf::RenderWindow& win, sf::Vector2f cam, float size) override;
};

} // namespace fornani
