
#pragma once

#include <djson/json.hpp>
#include <fornani/entity/Entity.hpp>
#include <memory>

namespace fornani {

class Chest : public Entity {
  public:
	Chest(automa::ServiceProvider& svc, dj::Json const& in);
	Chest(automa::ServiceProvider& svc, int type, int modifier, int id);
	Chest(automa::ServiceProvider& svc, int type, std::string tag, int modifier, int id);
	std::unique_ptr<Entity> clone() const override;
	void serialize(dj::Json& out) override;
	void unserialize(dj::Json const& in) override;
	void expose() override;
	void render(sf::RenderWindow& win, sf::Vector2f cam, float size) override;

  private:
	int m_type{};
	std::optional<std::string> m_tag{};
	int m_content_modifier{};
	automa::ServiceProvider* m_services;
};

} // namespace fornani
