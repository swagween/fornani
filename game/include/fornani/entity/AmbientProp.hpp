
#pragma once

#include <djson/json.hpp>
#include <fornani/components/CircleSensor.hpp>
#include <fornani/components/SteeringComponent.hpp>
#include <fornani/entity/Entity.hpp>
#include <memory>

namespace fornani {

enum class AmbientPropAttributes { foreground, destructible, audio };

struct AmbientPropParameters {
	AmbientPropParameters(dj::Json const& in);
	int num_frames{};
	float sensitivity{};
	float radius{};
	sf::Vector2i dimensions{};
	std::optional<std::string> destroy_effect{};
	std::optional<std::string> sound_effect{};
	util::BitFlags<AmbientPropAttributes> attributes{};
	sf::Vector2f offset{};
};

class AmbientProp : public Entity {
  public:
	AmbientProp(automa::ServiceProvider& svc, dj::Json const& in);
	AmbientProp(automa::ServiceProvider& svc, int channel, std::string_view tag);
	std::unique_ptr<Entity> clone() const;
	void serialize(dj::Json& out) override;
	void unserialize(dj::Json const& in) override;
	void expose() override;
	void update([[maybe_unused]] automa::ServiceProvider& svc, [[maybe_unused]] world::Map& map, [[maybe_unused]] SceneContext& context, [[maybe_unused]] player::Player& player) override;
	void render(sf::RenderWindow& win, sf::Vector2f cam, float size) override;

  private:
	int m_channel{};
	std::string m_tag{};
	std::optional<AmbientPropParameters> m_params{};
	components::SteeringComponent m_bob{};
	components::CircleSensor m_sensor{};
};

} // namespace fornani
