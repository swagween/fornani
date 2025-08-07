
#pragma once

#include <string_view>
#include "editor/canvas/Entity.hpp"

namespace pi {

class Light : public Entity {
  public:
	Light(fornani::automa::ServiceProvider& svc, dj::Json const& in);
	Light(fornani::automa::ServiceProvider& svc, int id, std::string_view type);
	std::unique_ptr<Entity> clone() const override;
	void serialize(dj::Json& out) override;
	void unserialize(dj::Json const& in) override;
	void expose() override;
	void render(sf::RenderWindow& win, sf::Vector2f cam, float size) override;

  private:
	std::string m_type{};
};

} // namespace pi
