
#pragma once

#include <cstdint>

namespace fornani {

namespace automa {
struct ServiceProvider;
}

namespace world {
class Map;
}

namespace player {
class Player;
}

namespace gui {
class Console;
}

namespace arms {
enum class Team : std::uint8_t;
}

namespace input {
class InputSystem;
}

namespace io {
class Loader;
}

struct SceneContext;

class SpriteBatch;
class Renderer;
class ResourceFinder;

} // namespace fornani
