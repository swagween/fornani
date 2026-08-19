
#pragma once

#include <fornani/core/Fwd.hpp>
#include <fornani/io/Logger.hpp>
#include <fornani/particle/Sparkler.hpp>
#include <fornani/physics/Shape.hpp>

namespace fornani::entity {
enum class BedFlags : std::uint8_t { active, engaged, slept_in };
class Bed {
  public:
	Bed(automa::ServiceProvider& svc, sf::Vector2f position, int style = 0, bool flipped = false);
	void update(automa::ServiceProvider& svc, world::Map& map, SceneContext& context, player::Player& player);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam);
	shape::Shape bounding_box{};

  private:
	vfx::Sparkler sparkler;
	util::BitFlags<BedFlags> flags{};
	util::Cooldown fadeout{200};
	sf::Sprite m_sprite;

	io::Logger m_logger{"Bed"};
};

} // namespace fornani::entity
