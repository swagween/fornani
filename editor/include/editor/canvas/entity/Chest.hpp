
#pragma once

#include <djson/json.hpp>
#include <memory>
#include "editor/canvas/Entity.hpp"

namespace pi {

class Chest : public Entity {
  public:
	Chest(fornani::automa::ServiceProvider& svc, dj::Json const& in);
	Chest(fornani::automa::ServiceProvider& svc, int type, int modifier, int id);
	std::unique_ptr<Entity> clone() const override;
	void serialize(dj::Json& out) override;
	void unserialize(dj::Json const& in) override;
	void expose() override;
	void render(sf::RenderWindow& win, sf::Vector2f cam, float size) override;

  private:
	int m_type{};
	int m_content_modifier{};
	fornani::automa::ServiceProvider* m_services;
};

} // namespace pi
