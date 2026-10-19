
#pragma once

#include <fornani/entity/Entity.hpp>
#include <fornani/physics/RegisteredCollider.hpp>
#include <fornani/utils/BitFlags.hpp>

namespace fornani {

enum class SpecialBlockAttributes : std::uint8_t { wallslide };

class SpecialBlock : public Entity {
  public:
	SpecialBlock(automa::ServiceProvider& svc, dj::Json const& in);
	SpecialBlock(automa::ServiceProvider& svc, world::Map& map, dj::Json const& in);
	SpecialBlock(automa::ServiceProvider& svc, sf::Vector2u dimensions, int id, std::string_view label, util::BitFlags<SpecialBlockAttributes> attributes = {});
	[[nodiscard]] virtual auto get_type() const -> std::string_view { return "default"; }
	void serialize(dj::Json& out) override;
	void unserialize(dj::Json const& in) override;
	void update([[maybe_unused]] automa::ServiceProvider& svc, [[maybe_unused]] world::Map& map, [[maybe_unused]] SceneContext& context, [[maybe_unused]] player::Player& player) override;
	void expose() override;
	void render(sf::RenderWindow& win, sf::Vector2f cam, float size) override;

	void init(automa::ServiceProvider& svc, dj::Json const& in);
	void on_hit(automa::ServiceProvider& svc, world::Map& map, arms::Projectile& proj);

	[[nodiscard]] auto get_collider() const -> shape::Collider& { return collider.value().get().get_reference(); }
	[[nodiscard]] auto has_collider() const -> bool { return collider.has_value(); }
	[[nodiscard]] auto ignore_updates() const -> bool;
	[[nodiscard]] auto is_wallslide() const -> bool { return m_attributes.test(SpecialBlockAttributes::wallslide); }

  private:
	// in-game use
	std::optional<shape::RegisteredCollider> owned_collider;
	std::optional<std::reference_wrapper<shape::RegisteredCollider>> collider;
	util::BitFlags<SpecialBlockAttributes> m_attributes{};
};

} // namespace fornani
