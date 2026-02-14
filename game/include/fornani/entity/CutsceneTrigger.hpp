
#pragma once

#include <fornani/entity/Entity.hpp>
#include <fornani/story/Quest.hpp>
#include <fornani/utils/BitFlags.hpp>
#include <fornani/physics/Shape.hpp>

namespace fornani {

enum class CutsceneTriggerFlags { activated, pushed };

class CutsceneTrigger : public Entity {
  public:
	CutsceneTrigger(automa::ServiceProvider& svc, dj::Json const& in);
	CutsceneTrigger(automa::ServiceProvider& svc, sf::Vector2u dimensions, int id);
	std::unique_ptr<Entity> clone() const override;
	void serialize(dj::Json& out) override;
	void unserialize(dj::Json const& in) override;
	void update([[maybe_unused]] automa::ServiceProvider& svc, [[maybe_unused]] world::Map& map, [[maybe_unused]] std::optional<std::unique_ptr<gui::Console>>& console, [[maybe_unused]] player::Player& player) override;
	void expose() override;
	void render(sf::RenderWindow& win, sf::Vector2f cam, float size) override;

	[[nodiscard]] auto is_activated() const -> bool { return m_flags.test(CutsceneTriggerFlags::activated); }
	[[nodiscard]] auto is_pushed() const -> bool { return m_flags.test(CutsceneTriggerFlags::pushed); }

  private:
	std::vector<QuestContingency> m_contingencies{};

	// in-game use
	shape::Shape m_bounding_box{};
	util::BitFlags<CutsceneTriggerFlags> m_flags{};
};

} // namespace fornani
