
#pragma once

#include <fornani/entity/Entity.hpp>
#include <fornani/physics/Shape.hpp>
#include <fornani/utils/Flaggable.hpp>

namespace fornani {

enum class WaterAttributes { toxic };
enum class WaterFlags { splashed };

class Water : public Entity, public Flaggable<WaterFlags> {
  public:
	Water(automa::ServiceProvider& svc, dj::Json const& in);
	Water(automa::ServiceProvider& svc, sf::Vector2u dimensions, int id, bool toxic = false);
	void serialize(dj::Json& out) override;
	void unserialize(dj::Json const& in) override;
	void update([[maybe_unused]] automa::ServiceProvider& svc, [[maybe_unused]] world::Map& map, [[maybe_unused]] std::optional<std::unique_ptr<gui::Console>>& console, [[maybe_unused]] player::Player& player) override;
	void expose() override;
	void render(sf::RenderWindow& win, sf::Vector2f cam, float size) override;

  private:
	sf::RenderTexture m_texture{};
	sf::RenderTexture m_surface_texture{};
	shape::Shape m_bounding_box{};
	Drawable m_surface;

	util::FloatCounter m_wave_timer{};
	sf::Vector2f m_wave_shift{};

	util::BitFlags<WaterAttributes> m_attributes{};
};

} // namespace fornani
