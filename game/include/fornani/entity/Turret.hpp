
#pragma once

#include <fornani/entity/Entity.hpp>
#include <fornani/utils/Direction.hpp>

namespace fornani {

enum class TurretType { laser, projectile };
enum class TurretPattern { constant, repeater, triggerable };

class Turret : public Entity {
  public:
	Turret(automa::ServiceProvider& svc, dj::Json const& in);
	Turret(automa::ServiceProvider& svc, int id, TurretType type, TurretPattern pattern, CardinalDirection dir);
	void init();
	std::unique_ptr<Entity> clone() const override;
	void serialize(dj::Json& out) override;
	void unserialize(dj::Json const& in) override;
	void expose() override;
	void update([[maybe_unused]] automa::ServiceProvider& svc, [[maybe_unused]] world::Map& map, [[maybe_unused]] std::optional<std::unique_ptr<gui::Console>>& console, [[maybe_unused]] player::Player& player) override;
	void render(sf::RenderWindow& win, sf::Vector2f cam, float size) override;

  private:
	TurretType m_type{};
	TurretPattern m_pattern{};
	CardinalDirection m_direction{};

	util::Cooldown m_rate{};
};

} // namespace fornani
