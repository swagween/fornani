
#pragma once

#include "editor/canvas/Entity.hpp"

namespace pi {

class Platform : public Entity {
  public:
	Platform(sf::Vector2u dim, int extent, std::string type, float start);

	std::unique_ptr<Entity> clone() const override;
	void serialize(dj::Json& out) override;
	void unserialize(dj::Json& in) override;
	void render(sf::RenderWindow& win, sf::Vector2<float> cam) override;

  private:
	int extent{};
	std::string type{};
	float start{};
};
} // namespace pi
