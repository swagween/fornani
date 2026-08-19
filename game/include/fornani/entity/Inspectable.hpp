
#pragma once

#include <fornani/core/Common.hpp>
#include <fornani/entity/Entity.hpp>
#include <fornani/events/Subscription.hpp>
#include <fornani/gui/console/Message.hpp>
#include <fornani/utils/ID.hpp>

namespace fornani {

enum class InspectableAttributes : std::uint8_t { activate_on_contact, instant };
enum class InspectableFlags : std::uint8_t { hovered, hovered_trigger, activated, destroy, engaged, can_engage };

class Inspectable : public Entity {
  public:
	Inspectable(automa::ServiceProvider& svc, dj::Json const& in);
	Inspectable(automa::ServiceProvider& svc, std::vector<std::vector<gui::BasicMessage>> suite, std::vector<std::vector<gui::BasicMessage>> responses, bool activate_on_contact, std::string key, int alternates, bool instant);

	std::unique_ptr<Entity> clone() const override;
	void serialize(dj::Json& out) override;
	void unserialize(dj::Json const& in) override;
	void expose() override;
	void render(sf::RenderWindow& win, sf::Vector2f cam, float size) override;

	// gameplay
	void update([[maybe_unused]] automa::ServiceProvider& svc, [[maybe_unused]] world::Map& map, [[maybe_unused]] SceneContext& context, [[maybe_unused]] player::Player& player);
	void destroy_me(automa::ServiceProvider& svc);
	void set_index(int to) { m_index = to; }
	[[nodiscard]] auto destroyed() const -> bool { return flags.test(InspectableFlags::destroy); }
	[[nodiscard]] auto get_label() const -> std::string { return m_key; }

  private:
	// entity
	bool m_activate_on_contact{};
	bool m_instant{};
	std::string m_key{};
	std::vector<std::vector<gui::BasicMessage>> m_suites{};
	std::vector<std::vector<gui::BasicMessage>> m_responses{};
	int m_alternates{};

	// gameplay
	shape::Shape bounding_box{};
	std::shared_ptr<Slot const> slot{std::make_shared<Slot const>()};
	int alternates{};
	int current_alt{};
	int m_index{};
	util::BitFlags<InspectableAttributes> attributes{};
	util::BitFlags<InspectableFlags> flags{};
	util::Cooldown m_indicator_cooldown{1300};
	gui::OutputType m_output{};
};

} // namespace fornani
