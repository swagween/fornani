
#pragma once

#include <fornani/entity/Entity.hpp>

namespace fornani {

class Interactable : public Entity {
  public:
	Interactable(automa::ServiceProvider& svc, dj::Json const& in);
	Interactable(automa::ServiceProvider& svc, std::string_view tag, int channel);
	std::unique_ptr<Entity> clone() const override;
	void serialize(dj::Json& out) override;
	void unserialize(dj::Json const& in) override;
	void expose() override;
	void update([[maybe_unused]] automa::ServiceProvider& svc, [[maybe_unused]] world::Map& map, [[maybe_unused]] SceneContext& context, [[maybe_unused]] player::Player& player) override;
	void render(sf::RenderWindow& win, sf::Vector2f cam, float size) override;

	[[nodiscard]] auto get_tag() const -> std::string_view { return m_tag; }

  private:
	std::string m_tag{};
	int m_channel{};
	sf::Vector2f m_offset{};
};

} // namespace fornani
