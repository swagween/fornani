
#pragma once

#include <fornani/components/PhysicsComponent.hpp>
#include <fornani/physics/Shape.hpp>
#include <fornani/systems/Register.hpp>
#include <fornani/utils/Polymorphic.hpp>

namespace fornani::world {
class Map;
class Tile;
} // namespace fornani::world

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani::shape {

constexpr auto vicinity_pad_v = 33.f;

class Collider;

enum class ColliderFlags { changed, intangible };
enum class ColliderType { rectangle, circle };
enum class ColliderAttributes { fixed, soft, top_only };
enum class CollisionExclusions { circle };

class ICollider : public Polymorphic {
  public:
	ICollider(sf::Vector2f dimensions);
	virtual void handle_collision(ICollider& other);
	virtual void update(automa::ServiceProvider& svc, bool simple = false);
	virtual void handle_map_collision(world::Map& map);
	virtual void handle_map_collision(world::Tile const& tile);
	virtual void detect_map_collision(world::Map& map);
	virtual void handle_collision(Shape& shape, bool soft = false);
	virtual bool handle_collider_collision(Shape const& collider, bool soft = false, sf::Vector2f velocity = {}, float force = 0.01f); // returns true if grounded on collider
	virtual void handle_collider_collision(Collider const& collider, bool soft = false, bool momentum = false);
	virtual void render(sf::RenderWindow& win, sf::Vector2f cam);

	void register_chunks(world::Map& map);
	void set_top_only() { m_attributes.set(ColliderAttributes::top_only); }
	void set_flag(ColliderFlags const to_set, bool on = true) { on ? m_flags.set(to_set) : m_flags.reset(to_set); }
	void set_exclusion(CollisionExclusions const to_set, bool on = true) { on ? m_exclusions.set(to_set) : m_exclusions.reset(to_set); }
	void set_attribute(ColliderAttributes const to_set) { m_attributes.set(to_set); }

	std::vector<int> compute_chunks(world::Map& map);
	std::vector<int> get_chunks() const;

	[[nodiscard]] auto was_changed() const -> bool { return m_flags.test(ColliderFlags::changed); }
	[[nodiscard]] auto is_intangible() const -> bool { return m_flags.test(ColliderFlags::intangible); }
	[[nodiscard]] auto has_attribute(ColliderAttributes const test) const -> bool { return m_attributes.test(test); }
	[[nodiscard]] auto has_exclusion(CollisionExclusions const test) const -> bool { return m_exclusions.test(test); }
	[[nodiscard]] auto is(ColliderType const test) const -> bool { return p_type == test; }
	[[nodiscard]] auto get_vicinity_rect() const -> sf::FloatRect { return p_vicinity.as_rect(); }

	components::PhysicsComponent physics{};

	// debug
	std::vector<std::string> print_chunks();

  protected:
	shape::Shape p_vicinity;
	ColliderType p_type{};

  private:
	Register<int> m_chunks;
	util::BitFlags<ColliderAttributes> m_attributes{};
	util::BitFlags<ColliderFlags> m_flags{};
	util::BitFlags<CollisionExclusions> m_exclusions{};
};

} // namespace fornani::shape
