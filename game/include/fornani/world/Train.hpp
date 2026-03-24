
#pragma once

#include <fornani/components/SteeringComponent.hpp>
#include <fornani/core/Fwd.hpp>
#include <fornani/graphics/Animatable.hpp>
#include <fornani/particle/Chain.hpp>
#include <fornani/utils/Cooldown.hpp>
#include <fornani/utils/Flaggable.hpp>

namespace fornani::world {

enum class TrainFlags { away, approaching, stopped, leaving };

struct TrainCar {
	int index{};
	sf::Vector2f position{};
};

class Train : public Animatable, public vfx::Chain, public Flaggable<TrainFlags> {
  public:
	Train(automa::ServiceProvider& svc, sf::Vector2i platform_position, int style = 0);
	void update(automa::ServiceProvider& svc, Map& map, player::Player& player);
	void render(sf::RenderWindow& win, sf::Vector2f cam);

  private:
	void debug();

  private:
	Animatable m_wheels;
	sf::Vector2i m_platform_position;
	components::SteeringComponent m_steering{};
	std::vector<TrainCar> m_cars{};
	util::Cooldown m_prepare_arrive;
	util::Cooldown m_prepare_leave;
	util::Cooldown m_play_horn;
	int m_horn_selection{};
	int m_style;
};

} // namespace fornani::world
