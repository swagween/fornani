
#pragma once

#include <fornani/graphics/Animatable.hpp>
#include <fornani/utils/BitFlags.hpp>
#include <fornani/utils/IWorldPositionable.hpp>

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani::entity {

enum class AnimatorFlags { foreground };

class Animator : public Animatable, IWorldPositionable {
  public:
	Animator(automa::ServiceProvider& svc, std::string_view label, int lookup, sf::Vector2i pos, bool foreground = false);
	void update();
	void render(sf::RenderWindow& win, sf::Vector2f cam);
	[[nodiscard]] auto is_foreground() const -> bool { return m_flags.test(AnimatorFlags::foreground); }

  private:
	util::BitFlags<AnimatorFlags> m_flags{};
};

} // namespace fornani::entity
