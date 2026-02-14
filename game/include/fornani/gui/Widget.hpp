
#pragma once

#include <SFML/Graphics.hpp>
#include <fornani/components/SteeringComponent.hpp>
#include <fornani/entities/animation/StateMachine.hpp>
#include <fornani/graphics/Animatable.hpp>
#include <fornani/utils/Cooldown.hpp>

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani::player {
class Player;
}

namespace fornani::gui {

enum class WidgetState { added, neutral, taken, gone };
enum class WidgetType { basic, secondary };

class Widget : public Animatable, public StateMachine<WidgetState> {
  public:
	Widget(automa::ServiceProvider& svc, std::string_view tag, sf::Vector2i dim, int index, sf::Vector2f root);
	void update(automa::ServiceProvider& svc, int max);
	void render(sf::RenderWindow& win);
	void shake() { m_shaking.start(); }
	void set_physics_offset(sf::Vector2f const offset) { m_steering.physics.position += offset; }
	void set_state(WidgetState const to) { set(to); }
	void set_root(sf::Vector2f const to) { m_root = to; }
	void set_offset(sf::Vector2f const to) { m_offset = to; }
	void set_type(WidgetType const to) { m_type = to; }

  private:
	WidgetType m_type{};
	components::SteeringComponent m_steering{};
	sf::Vector2f m_root{};
	sf::Vector2f m_offset{};
	int maximum{};
	int index{};
	util::Cooldown m_shaking;
};

} // namespace fornani::gui
