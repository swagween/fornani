
#pragma once

#include <fornani/entities/scenery/ChampionJ5.hpp>
#include <fornani/graphics/Animatable.hpp>
#include <fornani/story/Cutscene.hpp>

namespace fornani {

enum class ReturnToBaseFlags { done };

class ReturnToBase final : public Cutscene {
  public:
	explicit ReturnToBase(automa::ServiceProvider& svc);
	void update(automa::ServiceProvider& svc, SceneContext& context, world::Map& map, player::Player& player) override;
	void render(sf::RenderWindow& win, sf::Vector2f cam) override;

  private:
	util::Cooldown m_intro;
	std::optional<ChampionJ5> m_champion;

	util::BitFlags<ReturnToBaseFlags> m_flags{};

	sf::Vector2f m_exit_point{};
};

} // namespace fornani
