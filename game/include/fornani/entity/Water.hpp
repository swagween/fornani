
#pragma once

#include <fornani/entity/Entity.hpp>
#include <fornani/particle/Sparkler.hpp>
#include <fornani/physics/Shape.hpp>
#include <fornani/utils/Flaggable.hpp>

namespace fornani {

enum class WaterType { normal, curative, toxic };
enum class WaterFlags { splashed, touched };

class Water : public Entity, public Flaggable<WaterFlags> {
  public:
	Water(automa::ServiceProvider& svc, dj::Json const& in);
	Water(automa::ServiceProvider& svc, sf::Vector2u dimensions, int id, WaterType type);
	void serialize(dj::Json& out) override;
	void unserialize(dj::Json const& in) override;
	void update([[maybe_unused]] automa::ServiceProvider& svc, [[maybe_unused]] world::Map& map, [[maybe_unused]] SceneContext& context, [[maybe_unused]] player::Player& player) override;
	void expose() override;
	void render(sf::RenderWindow& win, sf::Vector2f cam, float size) override;

	[[nodiscard]] auto get_i_type() const -> int { return static_cast<int>(m_type); }

	shape::Shape const& get_bounding_box() const { return m_bounding_box; }

  private:
	sf::RenderTexture m_texture{};
	sf::RenderTexture m_surface_texture{};
	sf::BlendMode m_mode;
	shape::Shape m_bounding_box{};
	util::Cooldown m_replenish_cooldown;

	util::FloatCounter m_wave_timer{};
	sf::Vector2f m_wave_shift{};

	WaterType m_type{};

	std::optional<vfx::Sparkler> m_sparkler{};
};

} // namespace fornani
