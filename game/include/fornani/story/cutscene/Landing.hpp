
#pragma once

#include <fornani/entities/scenery/ChampionJ5.hpp>
#include <fornani/graphics/Animatable.hpp>
#include <fornani/story/Cutscene.hpp>

namespace fornani {

constexpr auto landing_id_v = 997;

enum class LandingFlags : std::uint8_t { done, started };

class Landing final : public Cutscene {
  public:
	explicit Landing(automa::ServiceProvider& svc, world::Map& map);
	void update(automa::ServiceProvider& svc, SceneContext& context, world::Map& map, player::Player& player) override;
	void render(sf::RenderWindow& win, sf::Vector2f cam) override;

  private:
	util::Cooldown m_landed;
	std::optional<ChampionJ5> m_champion;
	sf::Vector2f m_jitter{};
	int m_room{};

	util::BitFlags<LandingFlags> m_flags{};

	sf::Vector2f m_exit_point{};
};

} // namespace fornani
