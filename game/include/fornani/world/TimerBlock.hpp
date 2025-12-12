
#pragma once

#include <SFML/Graphics.hpp>
#include <fornani/graphics/Drawable.hpp>
#include <fornani/io/Logger.hpp>
#include <fornani/utils/BitFlags.hpp>
#include <fornani/utils/IWorldPositionable.hpp>
#include <fornani/physics/Shape.hpp>

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani::player {
class Player;
}

namespace fornani::world {

class Map;

enum class TimerBlockType { start, finish };
enum class TimerBlockFlags { reached, triggered };

class TimerBlock : public Drawable, public IWorldPositionable {
  public:
	TimerBlock(automa::ServiceProvider& svc, sf::Vector2<std::uint32_t> pos, TimerBlockType type, int id);
	void update(automa::ServiceProvider& svc, Map& map, player::Player& player);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam);
	void trigger() { m_flags.set(TimerBlockFlags::triggered); }

	[[nodiscard]] auto get_id() const -> int { return m_id; }
	[[nodiscard]] auto get_type() const -> TimerBlockType { return m_type; }

  private:
	int m_id{};
	shape::Shape m_bounds{};
	TimerBlockType m_type{};
	util::BitFlags<TimerBlockFlags> m_flags{};

	io::Logger m_logger{"World"};
};

} // namespace fornani::world
