
#pragma once

#include <fornani/entity/Entity.hpp>

namespace fornani {

class Platform : public Entity {
  public:
	Platform(automa::ServiceProvider& svc, dj::Json const& in);
	Platform(automa::ServiceProvider& svc, sf::Vector2u dim, int extent, std::string type, float start);

	std::unique_ptr<Entity> clone() const override;
	void serialize(dj::Json& out) override;
	void unserialize(dj::Json const& in) override;
	void expose() override;
	void render(sf::RenderWindow& win, sf::Vector2f cam, float size) override;

  private:
	void init(automa::ServiceProvider& svc);
	int extent{};
	std::string type{};
	float start{};

	sf::RectangleShape m_track{};
};
} // namespace fornani
