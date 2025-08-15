
#pragma once

#include "editor/canvas/Entity.hpp"

#include <djson/json.hpp>

#include <memory>

namespace pi {

class Animator : public Entity {
  public:
	Animator(fornani::automa::ServiceProvider& svc, dj::Json const& in);
	Animator(fornani::automa::ServiceProvider& svc, int id, std::string_view label);
	std::unique_ptr<Entity> clone() const override;
	void serialize(dj::Json& out) override;
	void unserialize(dj::Json const& in) override;
	void expose() override;
	void render(sf::RenderWindow& win, sf::Vector2f cam, float size) override;

  private:
	std::string m_label{};
	bool m_foreground{};
};

} // namespace pi
