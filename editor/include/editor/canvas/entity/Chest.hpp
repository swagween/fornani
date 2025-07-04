
#pragma once

#include <djson/json.hpp>
#include <memory>
#include "editor/canvas/Entity.hpp"

namespace pi {

class Chest : public Entity {
  public:
	Chest();
	Chest(int type, int modifier, int id);
	std::unique_ptr<Entity> clone() const override;
	void serialize(dj::Json& out) override;
	void unserialize(dj::Json const& in) override;
	void expose() override;
	void render(sf::RenderWindow& win, sf::Vector2<float> cam, float size) override;

  private:
	int m_type{};
	int m_content_modifier{};
};

} // namespace pi
