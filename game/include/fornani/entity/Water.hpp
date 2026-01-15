
#pragma once

#include <fornani/entity/Entity.hpp>
#include <fornani/physics/Shape.hpp>
#include <fornani/utils/Flaggable.hpp>

namespace fornani {

enum class WaterFlags { toxic };

class Water : public Entity, public Flaggable<WaterFlags> {
  public:
	Water(automa::ServiceProvider& svc, dj::Json const& in);
	Water(automa::ServiceProvider& svc, sf::Vector2u dimensions, int id, bool toxic = false);
	std::unique_ptr<Entity> clone() const override;
	void serialize(dj::Json& out) override;
	void unserialize(dj::Json const& in) override;
	void update([[maybe_unused]] automa::ServiceProvider& svc, [[maybe_unused]] world::Map& map, [[maybe_unused]] std::optional<std::unique_ptr<gui::Console>>& console, [[maybe_unused]] player::Player& player) override;
	void expose() override;
	void render(sf::RenderWindow& win, sf::Vector2f cam, float size) override;

  private:
	shape::Shape m_bounding_box{};
};

} // namespace fornani
