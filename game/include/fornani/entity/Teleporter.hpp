
#pragma once

#include <fornani/components/CircleSensor.hpp>
#include <fornani/entity/Entity.hpp>
#include <fornani/utils/Cooldown.hpp>
#include <fornani/utils/Direction.hpp>

namespace fornani {

enum class TeleporterFlags {};

class Teleporter : public Entity {
  public:
	Teleporter(automa::ServiceProvider& svc, dj::Json const& in);
	Teleporter(automa::ServiceProvider& svc, int id, int type);
	std::unique_ptr<Entity> clone() const override;
	void serialize(dj::Json& out) override;
	void unserialize(dj::Json const& in) override;
	void expose() override;
	void update([[maybe_unused]] automa::ServiceProvider& svc, [[maybe_unused]] world::Map& map, [[maybe_unused]] SceneContext& context, [[maybe_unused]] player::Player& player) override;
	void render(sf::RenderWindow& win, sf::Vector2f cam, float size) override;
	void receive() { m_receiving.start(); }

	[[nodiscard]] auto get_direction() const -> CardinalDirection { return m_direction; }

  private:
	int m_type{};
	CardinalDirection m_direction{};
	util::Cooldown m_sending;
	util::Cooldown m_receiving;
	components::CircleSensor m_sensor;
};

} // namespace fornani
