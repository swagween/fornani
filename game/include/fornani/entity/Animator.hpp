
#pragma once

#include <djson/json.hpp>
#include <fornani/entity/Entity.hpp>
#include <memory>

namespace fornani {

class Animator : public Entity {
  public:
	Animator(automa::ServiceProvider& svc, dj::Json const& in);
	Animator(automa::ServiceProvider& svc, int id, std::string_view label);
	std::unique_ptr<Entity> clone() const override;
	void serialize(dj::Json& out) override;
	void unserialize(dj::Json const& in) override;
	void expose() override;
	void render(sf::RenderWindow& win, sf::Vector2f cam, float size) override;

  private:
	std::string m_label{};
	bool m_foreground{};
};

} // namespace fornani
