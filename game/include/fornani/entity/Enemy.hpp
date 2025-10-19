
#pragma once

#include <djson/json.hpp>
#include <fornani/entity/Entity.hpp>
#include <memory>

namespace fornani {

class Enemy : public Entity {
  public:
	Enemy(automa::ServiceProvider& svc, dj::Json const& in);
	Enemy(automa::ServiceProvider& svc, int id, int variant = 0);
	std::unique_ptr<Entity> clone() const override;
	void serialize(dj::Json& out) override;
	void unserialize(dj::Json const& in) override;
	void expose() override;
	void render(sf::RenderWindow& win, sf::Vector2f cam, float size) override;

  private:
	int m_variant{};
	sf::Vector2i m_start_direction{};
};

} // namespace fornani
