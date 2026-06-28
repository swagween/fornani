
#pragma once

#include <SFML/Graphics.hpp>
#include <fornani/entity/Entity.hpp>
#include <fornani/physics/RegisteredCollider.hpp>
#include <fornani/utils/BitFlags.hpp>

namespace fornani {

enum class DestructibleState { solid, destroyed };
enum class DestructibleAttributes { inverse, enemy_clear, unlit };

class Destructible : public Entity {
  public:
	Destructible(automa::ServiceProvider& svc, dj::Json const& in);
	Destructible(automa::ServiceProvider& svc, world::Map& map, dj::Json const& in);
	Destructible(automa::ServiceProvider& svc, int id, util::BitFlags<DestructibleAttributes> attributes);
	void serialize(dj::Json& out) override;
	void unserialize(dj::Json const& in) override;
	void expose() override;
	void update([[maybe_unused]] automa::ServiceProvider& svc, [[maybe_unused]] world::Map& map, [[maybe_unused]] SceneContext& context, [[maybe_unused]] player::Player& player) override;
	void render(sf::RenderWindow& win, sf::Vector2f cam, float size) override;
	void render(sf::RenderTexture& tex, sf::Vector2f cam);

	void init(automa::ServiceProvider& svc, dj::Json const& in);
	void on_hit(automa::ServiceProvider& svc, world::Map& map, arms::Projectile& proj);
	shape::Shape& get_bounding_box();

	[[nodiscard]] auto get_id() const -> int { return quest_id; }
	[[nodiscard]] auto is_unlit() const -> bool { return m_attributes.test(DestructibleAttributes::unlit); }
	[[nodiscard]] auto is_inverse() const -> bool { return m_attributes.test(DestructibleAttributes::inverse); }
	[[nodiscard]] auto is_enemy_clear() const -> bool { return m_attributes.test(DestructibleAttributes::enemy_clear); }
	[[nodiscard]] auto is_solid() const -> bool { return static_cast<DestructibleState>(m_state) == DestructibleState::solid; }
	[[nodiscard]] auto is_destroyed() const -> bool { return static_cast<DestructibleState>(m_state) == DestructibleState::destroyed; }
	[[nodiscard]] auto ignore_updates() const -> bool;
	[[nodiscard]] auto get_global_center() const -> sf::Vector2f { return collider.value().get().get_reference().get_center(); }
	[[nodiscard]] auto get_collider() const -> shape::Collider& { return collider.value().get().get_reference(); }
	[[nodiscard]] auto has_collider() const -> bool { return collider.has_value(); }

  private:
	int quest_id{};
	std::optional<shape::RegisteredCollider> owned_collider;
	std::optional<std::reference_wrapper<shape::RegisteredCollider>> collider;
	int m_state{};
	util::BitFlags<DestructibleAttributes> m_attributes{};
};

} // namespace fornani
