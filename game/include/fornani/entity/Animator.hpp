
#pragma once

#include <djson/json.hpp>
#include <fornani/entity/Entity.hpp>
#include <memory>

namespace fornani {

class Animator : public Entity {
  public:
	Animator(automa::ServiceProvider& svc, dj::Json const& in);
	Animator(automa::ServiceProvider& svc, int id, std::string_view label);
	std::unique_ptr<Entity> clone() const override;
	void serialize(dj::Json& out) override;
	void unserialize(dj::Json const& in) override;
	void expose() override;
	void update([[maybe_unused]] automa::ServiceProvider& svc, [[maybe_unused]] world::Map& map, [[maybe_unused]] SceneContext& context, [[maybe_unused]] player::Player& player) override;
	void render(sf::RenderWindow& win, sf::Vector2f cam, float size) override;
	void render(sf::RenderTexture& tex, sf::Vector2f cam);

	[[nodiscard]] auto is_foreground() const -> bool { return m_foreground; }

  private:
	std::string m_label{};
	bool m_foreground{};
};

} // namespace fornani
