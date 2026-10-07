
#pragma once

#include <djson/json.hpp>
#include <fornani/components/CircleSensor.hpp>
#include <fornani/components/SteeringComponent.hpp>
#include <fornani/entities/packages/Health.hpp>
#include <fornani/entity/Entity.hpp>
#include <fornani/graphics/Color.hpp>
#include <fornani/particle/Emitter.hpp>
#include <memory>

namespace fornani {

class FlatShader;

enum class AmbientPropAttributes : std::uint8_t { foreground, audio };
enum class AmbientPropFlags : std::uint8_t { interactable, flat_shaded, light_shaded };

struct AmbientColor : public Color {
	AmbientColor(dj::Json const& in, int index) : Color{in}, tile_index{index} {}
	int tile_index;
};

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
	std::vector<AmbientColor> hues{};
	sf::Vector2f shift_range{};
	float depth_multiplier{0.1f};
	std::vector<std::string> animations{};
};

struct AmbientPropVariables {
	float depth{};
};

class AmbientProp : public Entity {
  public:
	AmbientProp(automa::ServiceProvider& svc, dj::Json const& in);
	AmbientProp(automa::ServiceProvider& svc, int channel, std::string_view tag, float depth = 0.f);
	std::unique_ptr<Entity> clone() const;
	void serialize(dj::Json& out) override;
	void unserialize(dj::Json const& in) override;
	void expose() override;
	void update([[maybe_unused]] automa::ServiceProvider& svc, [[maybe_unused]] world::Map& map, [[maybe_unused]] SceneContext& context, [[maybe_unused]] player::Player& player) override;
	void render(sf::RenderWindow& win, sf::Vector2f cam, float size) override;
	void render(sf::RenderWindow& win, sf::RenderTexture& tex, sf::Vector2f cam);
	void flat_shade(sf::RenderWindow& win, sf::Vector2f cam, FlatShader& shader);

	[[nodiscard]] auto is_foreground() const -> bool { return m_params ? m_params->attributes.test(AmbientPropAttributes::foreground) : false; }
	[[nodiscard]] auto is_in_front() const -> bool { return m_variables.depth > 0.f; }
	[[nodiscard]] auto is_interactable() const -> bool { return m_flags.test(AmbientPropFlags::interactable); }
	[[nodiscard]] auto is_flat_shaded() const -> bool { return m_flags.test(AmbientPropFlags::flat_shaded); }
	[[nodiscard]] auto is_light_shaded() const -> bool { return m_flags.test(AmbientPropFlags::light_shaded); }
	[[nodiscard]] auto is_destructible() const -> bool { return m_health.has_value(); }
	[[nodiscard]] auto is_destroyed() const -> bool { return is_destructible() ? m_health->is_dead() : false; }
	[[nodiscard]] auto get_depth() const -> float { return m_variables.depth; }

  private:
	void init(automa::ServiceProvider& svc);
	[[nodiscard]] auto generate_position(sf::RenderWindow& win, sf::Vector2f cam) const -> sf::Vector2f;

  private:
	int m_channel{};
	std::optional<Health> m_health{};
	AmbientPropVariables m_variables{};
	util::BitFlags<AmbientPropFlags> m_flags{};
	std::string m_tag{};
	std::optional<AmbientPropParameters> m_params{};
	components::SteeringComponent m_bob{};
	components::CircleSensor m_sensor{};
	util::Cooldown m_emitter_cooldown{};
	sf::Color m_color{};
};

} // namespace fornani
