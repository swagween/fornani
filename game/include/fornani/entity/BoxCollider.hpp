
#pragma once

#include <fornani/entity/Entity.hpp>
#include <fornani/physics/Collider.hpp>
#include <fornani/story/Quest.hpp>
#include <fornani/utils/BitFlags.hpp>

namespace fornani {

enum class BoxColliderAttributes : std::uint8_t { wallslide };

class BoxCollider : public Entity {
  public:
	BoxCollider(automa::ServiceProvider& svc, dj::Json const& in);
	BoxCollider(automa::ServiceProvider& svc, sf::Vector2u dimensions, int id, util::BitFlags<BoxColliderAttributes> attributes = {});
	std::unique_ptr<Entity> clone() const override;
	void serialize(dj::Json& out) override;
	void unserialize(dj::Json const& in) override;
	void update([[maybe_unused]] automa::ServiceProvider& svc, [[maybe_unused]] world::Map& map, [[maybe_unused]] SceneContext& context, [[maybe_unused]] player::Player& player) override;
	void expose() override;
	void render(sf::RenderWindow& win, sf::Vector2f cam, float size) override;

	[[nodiscard]] auto is_wallslide() const -> bool { return m_attributes.test(BoxColliderAttributes::wallslide); }

  private:
	// in-game use
	shape::Collider m_collider;
	util::BitFlags<BoxColliderAttributes> m_attributes{};
};

} // namespace fornani
