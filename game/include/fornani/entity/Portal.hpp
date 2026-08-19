
#pragma once

#include <fornani/components/SteeringComponent.hpp>
#include <fornani/entity/Entity.hpp>
#include <fornani/graphics/Transition.hpp>
#include <fornani/physics/Shape.hpp>
#include <fornani/utils/BitFlags.hpp>
#include <fornani/utils/Cooldown.hpp>
#include <fornani/utils/IWorldPositionable.hpp>

namespace fornani {

enum class PortalAttributes : std::uint8_t { activate_on_contact, already_open };
enum class PortalState : std::uint8_t { activated, ready, locked, unlocked, transitioning };
enum class PortalRenderState : std::uint8_t { closed, open, locked };
enum class PortalOrientation : std::uint8_t { top, bottom, left, right, central };

enum class CustomPortalAttributes : std::uint8_t { open_for_player, gravitate };
enum class CustomPortalFlags : std::uint8_t { opened, closed };

class Portal;

struct CustomPortalAnimation {
	CustomPortalAnimation(automa::ServiceProvider& svc, std::string_view tag);
	void update(automa::ServiceProvider& svc, player::Player& player, Portal& parent);
	Animatable animatable;
	std::vector<std::string> sounds{};
	std::string tag{};
	sf::Vector2f offset{};
	util::BitFlags<CustomPortalAttributes> attributes{};
	util::BitFlags<CustomPortalFlags> flags{};

  private:
	components::SteeringComponent m_player_steering{};
};

struct PortalSpecifications {
	bool activate_on_contact{};
	bool already_open{};
	int source_map_id{};
	int destination_map_id{};
	int channel{};
};

class Portal : public Entity {
  public:
	Portal(automa::ServiceProvider& svc, dj::Json const& in);
	Portal(automa::ServiceProvider& svc, sf::Vector2u dimensions, PortalSpecifications specs);
	Portal(automa::ServiceProvider& svc, sf::Vector2u dimensions, PortalSpecifications specs, std::string_view key);

	std::unique_ptr<Entity> clone() const override;
	void serialize(dj::Json& out) override;
	void unserialize(dj::Json const& in) override;
	void expose() override;
	void update([[maybe_unused]] automa::ServiceProvider& svc, [[maybe_unused]] world::Map& map, [[maybe_unused]] SceneContext& context, [[maybe_unused]] player::Player& player) override;
	void render(sf::RenderWindow& win, sf::Vector2f cam, float size) override;
	void render(automa::ServiceProvider& svc, sf::RenderTexture& tex, sf::Vector2f cam);
	void close() { m_render_state = PortalRenderState::closed; }
	void open() { m_opened_cooldown.start(); }

	[[nodiscard]] auto get_source() const -> int { return source_id; }
	[[nodiscard]] auto get_destination() const -> int { return destination_id; }
	[[nodiscard]] auto get_center() const -> sf::Vector2f { return bounding_box.get_center(); }
	[[nodiscard]] auto is_activate_on_contact() const -> bool { return m_attributes.test(PortalAttributes::activate_on_contact); }
	[[nodiscard]] auto is_already_open() const -> bool { return m_attributes.test(PortalAttributes::already_open); }
	[[nodiscard]] auto is_locked() const -> bool { return m_state.test(PortalState::locked); }
	[[nodiscard]] auto is_activated() const -> bool { return m_state.test(PortalState::activated); }
	[[nodiscard]] auto is_transitioning() const -> bool { return m_state.test(PortalState::transitioning); }
	[[nodiscard]] auto is_top_or_bottom() const -> bool { return is_bottom() || is_top(); }
	[[nodiscard]] auto is_left_or_right() const -> bool { return is_left() || is_right(); }
	[[nodiscard]] auto is_bottom() const -> bool { return m_orientation == PortalOrientation::bottom; }
	[[nodiscard]] auto is_top() const -> bool { return m_orientation == PortalOrientation::top; }
	[[nodiscard]] auto is_left() const -> bool { return m_orientation == PortalOrientation::left; }
	[[nodiscard]] auto is_right() const -> bool { return m_orientation == PortalOrientation::right; }
	[[nodiscard]] auto has_custom_animation() const -> bool { return m_custom_animation.has_value(); }

  public:
	shape::Shape bounding_box{};

  private:
	void change_states(automa::ServiceProvider& svc, int room_id, graphics::Transition& transition);

  private:
	int source_id{};
	int destination_id{};
	int channel{};
	std::optional<std::string> key_tag{};

	PortalOrientation m_orientation{};
	PortalRenderState m_render_state{};
	util::BitFlags<PortalAttributes> m_attributes{};
	util::BitFlags<PortalState> m_state{};
	util::Cooldown m_opened_cooldown;

	std::optional<CustomPortalAnimation> m_custom_animation{};

	automa::ServiceProvider* m_services;
};

} // namespace fornani
