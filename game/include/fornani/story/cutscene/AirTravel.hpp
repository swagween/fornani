
#pragma once

#include <fornani/entities/scenery/ChampionJ5.hpp>
#include <fornani/graphics/Animatable.hpp>
#include <fornani/story/Cutscene.hpp>

namespace fornani {

constexpr auto air_travel_id_v = 998;

enum class AirTravelFlags : std::uint8_t { done, started, unavailable, launched_console, home_base, no_go };

class AirTravel final : public Cutscene {
  public:
	explicit AirTravel(automa::ServiceProvider& svc, world::Map& map);
	void update(automa::ServiceProvider& svc, SceneContext& context, world::Map& map, player::Player& player) override;
	void render(sf::RenderWindow& win, sf::Vector2f cam) override;

  private:
	void travel_to_room(int id);

  private:
	util::Cooldown m_landed;
	std::optional<ChampionJ5> m_champion;
	sf::Vector2f m_jitter{};
	int m_room{};
	std::optional<int> m_target_room{};

	util::BitFlags<AirTravelFlags> m_flags{};

	sf::Vector2f m_exit_point{};
};

} // namespace fornani
