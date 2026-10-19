
#pragma once

#include <fornani/entity/SpecialBlock.hpp>
#include <fornani/utils/BitFlags.hpp>

namespace fornani {

class ExpanderBlock : public SpecialBlock {

	enum class ExpanderBlockAttributes : std::uint8_t { periodic, unlit };
	enum class ExpanderBlockFlags : std::uint8_t { expanded };

  public:
	ExpanderBlock(automa::ServiceProvider& svc, dj::Json const& in);
	ExpanderBlock(automa::ServiceProvider& svc, world::Map& map, dj::Json const& in);
	ExpanderBlock(automa::ServiceProvider& svc, sf::Vector2u dimensions, int id, std::string_view label, util::BitFlags<SpecialBlockAttributes> attributes = {});
	[[nodiscard]] auto get_type() const -> std::string_view override { return "expander"; }
	[[nodiscard]] auto is_unlit() const -> bool { return m_attributes.test(ExpanderBlockAttributes::unlit); }

	void serialize(dj::Json& out) override;
	void unserialize(dj::Json const& in) override;
	void update(automa::ServiceProvider& svc, world::Map& map, SceneContext& context, player::Player& player) override;
	void render(sf::RenderWindow& win, sf::Vector2f cam, float size) override;
	void render(sf::RenderTexture& tex, sf::Vector2f cam);

  private:
	util::BitFlags<ExpanderBlockAttributes> m_attributes{};
	util::BitFlags<ExpanderBlockFlags> m_flags{};
	util::Cooldown m_timer{360};
	float m_threshold{0.5f};
	float m_scale{1.f};
	float m_factor{3.f};
};

} // namespace fornani
