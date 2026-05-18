
#pragma once

#include <fornani/io/Logger.hpp>
#include <fornani/story/Cutscene.hpp>
#include <fornani/systems/Register.hpp>
#include <memory>
#include <vector>

namespace fornani::automa {
struct ServiceProvider;
}
namespace fornani::gui {
class Console;
}
namespace fornani::world {
class Map;
}
namespace fornani::player {
class Player;
}
namespace fornani {

struct CutsceneSpec {
	int id{};
	int special{};

	constexpr CutsceneSpec() = default;

	constexpr CutsceneSpec(int id_, int special_) : id{id_}, special{special_} {}
	constexpr CutsceneSpec(CutsceneSpec const&) = default;
	constexpr CutsceneSpec(CutsceneSpec&&) noexcept = default;

	constexpr CutsceneSpec& operator=(CutsceneSpec const&) = default;
	constexpr CutsceneSpec& operator=(CutsceneSpec&&) noexcept = default;

	constexpr bool operator==(CutsceneSpec const&) const = default;
	constexpr auto operator<=>(CutsceneSpec const&) const = default;
};

class CutsceneCatalog {
  public:
	CutsceneCatalog() = default;
	explicit CutsceneCatalog(automa::ServiceProvider& svc);
	void update(automa::ServiceProvider& svc, SceneContext& context, world::Map& map, player::Player& player);
	void push_cutscene(automa::ServiceProvider& svc, world::Map& map, player::Player& player, int id, int special_id = 0);

	std::vector<std::unique_ptr<Cutscene>> cutscenes{};

  private:
	Register<int> m_register{};
	io::Logger m_logger{"Cutscene"};
};

} // namespace fornani
