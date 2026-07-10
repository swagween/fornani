
#pragma once

#include <fornani/core/Fwd.hpp>
#include <fornani/entities/atmosphere/Dragonfly.hpp>
#include <fornani/entities/atmosphere/Firefly.hpp>
#include <fornani/graphics/SpriteBatch.hpp>
#include <memory>
#include <vector>

namespace fornani::vfx {
class Atmosphere {
  public:
	Atmosphere(automa::ServiceProvider& svc, world::Map& map, std::string_view type);
	void update(automa::ServiceProvider& svc, world::Map& map, player::Player& player);
	void render(Renderer& renderer);
	void debug();

  private:
	std::vector<std::unique_ptr<Firefly>> fireflies{};
	std::vector<std::unique_ptr<Dragonfly>> dragonflies{};

	SpriteBatch m_firefly_batch{};

	// debug
	struct {
		float seek{};
		float walk{};
		float evade{};
	} forces{};
};

} // namespace fornani::vfx
