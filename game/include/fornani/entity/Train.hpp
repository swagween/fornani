
#pragma once

#include <fornani/components/SteeringComponent.hpp>
#include <fornani/core/Fwd.hpp>
#include <fornani/entity/Entity.hpp>
#include <fornani/particle/Chain.hpp>
#include <fornani/story/Quest.hpp>
#include <fornani/utils/Cooldown.hpp>
#include <fornani/utils/Flaggable.hpp>

namespace fornani {

enum class TrainFlags { away, approaching, stopped, leaving };

struct TrainCar {
	int index{};
	sf::Vector2f position{};
};

class Train : public Entity, public Flaggable<TrainFlags> {
  public:
	Train(automa::ServiceProvider& svc, dj::Json const& in);
	Train(automa::ServiceProvider& svc);
	Train(automa::ServiceProvider& svc, int style);
	std::unique_ptr<Entity> clone() const override;
	void serialize(dj::Json& out) override;
	void unserialize(dj::Json const& in) override;
	void expose() override;
	void update([[maybe_unused]] automa::ServiceProvider& svc, [[maybe_unused]] world::Map& map, [[maybe_unused]] std::optional<std::unique_ptr<gui::Console>>& console, [[maybe_unused]] player::Player& player) override;
	void render(sf::RenderWindow& win, sf::Vector2f cam, float size) override;

	void init(automa::ServiceProvider& svc);

  private:
	void debug();

  private:
	// serializable
	std::optional<QuestContingencySet> m_contingencies{};
	int m_style{};
	//

	Animatable m_wheels;
	std::optional<vfx::Chain> m_chain{};
	components::SteeringComponent m_steering{};
	std::vector<TrainCar> m_cars{};
	util::Cooldown m_prepare_arrive;
	util::Cooldown m_prepare_leave;
	util::Cooldown m_play_horn;
	int m_horn_selection{};
};

} // namespace fornani
