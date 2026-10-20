
#pragma once

#include <fornani/components/PhysicsComponent.hpp>
#include <fornani/graphics/Animatable.hpp>
#include <fornani/story/Cutscene.hpp>
#include <fornani/utils/Shaker.hpp>

namespace fornani {

enum class FirstwindEscapePodFlags : std::uint8_t { done, started, launched_console, no_go };

class FirstwindEscapePod final : public Cutscene {
  public:
	explicit FirstwindEscapePod(automa::ServiceProvider& svc, world::Map& map);
	void update(automa::ServiceProvider& svc, SceneContext& context, world::Map& map, player::Player& player) override;
	void render(sf::RenderWindow& win, sf::Vector2f cam) override;
	void render_on_top(sf::RenderWindow& win, sf::Vector2f cam) override;

  private:
	sf::Vector2f m_jitter{};
	util::BitFlags<FirstwindEscapePodFlags> m_flags{};
	components::PhysicsComponent m_pod_physics{};
	Shaker m_shaker;

	std::optional<Animatable> m_pod{};
};

} // namespace fornani
