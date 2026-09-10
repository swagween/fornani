
#pragma once

#include <fornani/entities/animation/Animation.hpp>
#include <fornani/graphics/Animatable.hpp>
#include <fornani/io/Logger.hpp>
#include <fornani/physics/RegisteredCollider.hpp>
#include <optional>

namespace fornani::entity {

enum class MineState : std::uint8_t { live, dead, exploded };
enum class MineType : std::uint8_t { floating, fixed };
enum class MineAttributes : std::uint8_t { mercurial };

class Mine final : public Animatable {
  public:
	Mine(automa::ServiceProvider& svc, world::Map& map, MineType type);

	void update(automa::ServiceProvider& svc, world::Map& map, player::Player& player);
	void render(sf::RenderWindow& win, sf::Vector2f cam);
	void set_position(sf::Vector2f pos) { get_collider().set_position(pos); }
	void on_hit(automa::ServiceProvider& svc, world::Map& map, arms::Projectile& proj, player::Player& player);
	void explode(automa::ServiceProvider& svc, world::Map& map);

	shape::CircleCollider& get_collider() { return *m_collider.get_circle(); }
	void set_attribute(MineAttributes const to_set, bool on = true) { on ? m_attributes.set(to_set) : m_attributes.reset(to_set); }

	[[nodiscard]] auto is_exploded() const -> bool { return m_state == MineState::exploded; }

  private:
	shape::RegisteredCollider m_collider;

	MineState m_state{};
	MineType m_type{};
	util::BitFlags<MineAttributes> m_attributes{};

	io::Logger m_logger{"Mine"};
};

} // namespace fornani::entity
