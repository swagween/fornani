
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
enum class ColliderAttributes { fixed, soft, top_only };

class ICollider : public Polymorphic {
  public:
	ICollider(sf::Vector2f dimensions);
	virtual void handle_collision(ICollider& other);
	virtual void update(automa::ServiceProvider& svc, bool simple = false);
	virtual void handle_map_collision(world::Map& map);
	virtual void handle_map_collision(world::Tile const& tile);
	virtual void detect_map_collision(world::Map& map);
	virtual void handle_collision(Shape& shape, bool soft = false);
	virtual bool handle_collider_collision(Shape const& collider, bool soft = false, sf::Vector2f velocity = {}); // returns true if grounded on collider
	virtual void handle_collider_collision(Collider const& collider, bool soft = false, bool momentum = false);
	virtual void render(sf::RenderWindow& win, sf::Vector2f cam);

	void register_chunks(world::Map& map);
	void set_top_only() { m_attributes.set(ColliderAttributes::top_only); }
	void set_flag(ColliderFlags const to_set, bool on) { on ? m_flags.set(to_set) : m_flags.reset(to_set); }
	void set_attribute(ColliderAttributes const to_set) { m_attributes.set(to_set); }

	std::vector<int> compute_chunks(world::Map& map);
	std::vector<int> get_chunks() const;

	[[nodiscard]] auto was_changed() const -> bool { return m_flags.test(ColliderFlags::changed); }
	[[nodiscard]] auto has_attribute(ColliderAttributes const test) const -> bool { return m_attributes.test(test); }

	components::PhysicsComponent physics{};

	// debug
	std::vector<std::string> print_chunks();

  protected:
	shape::Shape p_vicinity;

  private:
	Register<int> m_chunks;
	util::BitFlags<ColliderAttributes> m_attributes{};
	util::BitFlags<ColliderFlags> m_flags{};
};

} // namespace fornani::shape
