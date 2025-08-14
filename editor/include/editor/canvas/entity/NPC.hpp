
#pragma once

#include "editor/canvas/Entity.hpp"

#include <djson/json.hpp>

#include <memory>

namespace pi {

class NPC : public Entity {
  public:
	NPC(fornani::automa::ServiceProvider& svc, dj::Json const& in);
	NPC(fornani::automa::ServiceProvider& svc, int id, std::string_view label);
	std::unique_ptr<Entity> clone() const override;
	void serialize(dj::Json& out) override;
	void unserialize(dj::Json const& in) override;
	void expose() override;
	void render(sf::RenderWindow& win, sf::Vector2f cam, float size) override;

  private:
	std::string m_label{};
};

} // namespace pi
