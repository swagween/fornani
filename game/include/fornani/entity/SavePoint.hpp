
#pragma once

#include <fornani/entity/Entity.hpp>
#include <fornani/graphics/Animatable.hpp>
#include <fornani/io/Logger.hpp>
#include <fornani/particle/Sparkler.hpp>
#include <fornani/utils/IWorldPositionable.hpp>
#include <fornani/utils/Shape.hpp>
#include <optional>

namespace fornani {

class SavePoint : public Entity {
  public:
	SavePoint(automa::ServiceProvider& svc, dj::Json const& in);
	SavePoint(automa::ServiceProvider& svc, int id);
	std::unique_ptr<Entity> clone() const override;
	void serialize(dj::Json& out) override;
	void unserialize(dj::Json const& in) override;
	void expose() override;
	void render(sf::RenderWindow& win, sf::Vector2f cam, float size) override;
	void update([[maybe_unused]] automa::ServiceProvider& svc, [[maybe_unused]] world::Map& map, [[maybe_unused]] std::optional<std::unique_ptr<gui::Console>>& console, [[maybe_unused]] player::Player& player) override;

  private:
	void save(automa::ServiceProvider& svc, player::Player& player);

	anim::Parameters m_anim_params;
	shape::Shape bounding_box{};
	shape::Shape proximity_box{};
	vfx::Sparkler sparkler;
	bool activated{};
	bool can_activate{true};
	int intensity{};
};

} // namespace fornani
