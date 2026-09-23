
#pragma once

#include <djson/json.hpp>
#include <fornani/components/CircleSensor.hpp>
#include <fornani/components/SteeringComponent.hpp>
#include <fornani/entity/Entity.hpp>
#include <fornani/particle/Emitter.hpp>
#include <memory>

namespace fornani {

class FlatShader;

enum class AmbientPropAttributes : std::uint8_t { foreground, destructible, audio };
enum class AmbientPropFlags : std::uint8_t { interactable, flat_shaded };

struct AmbientPropParameters {
	AmbientPropParameters(automa::ServiceProvider& svc, dj::Json const& in);
	int num_frames{};
	float sensitivity{};
	float radius{};
	sf::Vector2i dimensions{};
	std::optional<std::string> destroy_effect{};
	std::optional<std::string> sound_effect{};
	util::BitFlags<AmbientPropAttributes> attributes{};
	sf::Vector2f offset{};
	std::optional<vfx::EmitterParameters> emitter{};
	std::vector<sf::Color> hues{};
	sf::Vector2f shift_range{};
};

struct AmbientPropVariables {
	float depth{};
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
	void flat_shade(sf::RenderWindow& win, sf::Vector2f cam, FlatShader& shader);

	[[nodiscard]] auto is_foreground() const -> bool { return m_params ? m_params->attributes.test(AmbientPropAttributes::foreground) : false; }
	[[nodiscard]] auto is_in_front() const -> bool { return m_variables.depth > constants::tiny_value; }
	[[nodiscard]] auto is_interactable() const -> bool { return m_flags.test(AmbientPropFlags::interactable); }
	[[nodiscard]] auto is_flat_shaded() const -> bool { return m_flags.test(AmbientPropFlags::flat_shaded); }
	[[nodiscard]] auto get_depth() const -> float { return m_variables.depth; }

  private:
	int m_channel{};
	AmbientPropVariables m_variables{};
	util::BitFlags<AmbientPropFlags> m_flags{};
	std::string m_tag{};
	std::optional<AmbientPropParameters> m_params{};
	components::SteeringComponent m_bob{};
	components::CircleSensor m_sensor{};
	util::Cooldown m_emitter_cooldown{};
};

} // namespace fornani
