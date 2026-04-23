
#pragma once

#include <fornani/entities/scenery/ChampionJ5.hpp>
#include <fornani/graphics/Animatable.hpp>
#include <fornani/story/Cutscene.hpp>

namespace fornani {

enum class HaunchEscapeFlags { done, over };

class HaunchEscape final : public Cutscene {
  public:
	explicit HaunchEscape(automa::ServiceProvider& svc);
	void update(automa::ServiceProvider& svc, SceneContext& context, world::Map& map, player::Player& player) override;
	void render(sf::RenderWindow& win, sf::Vector2f cam) override;

  private:
	util::Cooldown m_intro;
	util::Cooldown m_dynamite;
	util::Cooldown m_bomb_tick;
	util::Cooldown m_heroes_exit;
	util::Cooldown m_outro;
	util::Cooldown m_champion_entry;
	std::optional<ChampionJ5> m_champion;

	util::BitFlags<HaunchEscapeFlags> m_flags{};
};

} // namespace fornani
