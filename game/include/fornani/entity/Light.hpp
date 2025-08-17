
#pragma once

#include <fornani/entity/Entity.hpp>
#include <string_view>

namespace fornani {

class Light : public Entity {
  public:
	Light(automa::ServiceProvider& svc, dj::Json const& in);
	Light(automa::ServiceProvider& svc, int id, std::string_view type);
	std::unique_ptr<Entity> clone() const override;
	void serialize(dj::Json& out) override;
	void unserialize(dj::Json const& in) override;
	void expose() override;
	void render(sf::RenderWindow& win, sf::Vector2f cam, float size) override;

  private:
	std::string m_type{};
};

} // namespace fornani
