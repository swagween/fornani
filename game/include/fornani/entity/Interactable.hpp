
#pragma once

#include <fornani/entity/Entity.hpp>

namespace fornani {

class Interactable : public Entity {

	enum class InteractableFlags : std::uint8_t { hidden };

  public:
	Interactable(automa::ServiceProvider& svc, dj::Json const& in);
	Interactable(automa::ServiceProvider& svc, std::string_view tag, int channel);
	std::unique_ptr<Entity> clone() const override;
	void serialize(dj::Json& out) override;
	void unserialize(dj::Json const& in) override;
	void expose() override;
	void update([[maybe_unused]] automa::ServiceProvider& svc, [[maybe_unused]] world::Map& map, [[maybe_unused]] SceneContext& context, [[maybe_unused]] player::Player& player) override;
	void render(sf::RenderWindow& win, sf::Vector2f cam, float size) override;
	void set_hidden(bool const to = true) { to ? m_flags.set(InteractableFlags::hidden) : m_flags.reset(InteractableFlags::hidden); }

	void set_animation(std::string_view to);

	[[nodiscard]] auto is_hidden() const -> bool { return m_flags.test(InteractableFlags::hidden); }
	[[nodiscard]] auto get_tag() const -> std::string_view { return m_tag; }

  private:
	std::string m_tag{};
	int m_channel{};
	sf::Vector2f m_offset{};
	util::BitFlags<InteractableFlags> m_flags{};
};

} // namespace fornani
