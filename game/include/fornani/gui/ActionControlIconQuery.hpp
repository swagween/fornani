#pragma once
#include <SFML/Graphics.hpp>
#include "fornani/setup/ControllerMap.hpp"

namespace fornani::automa {
struct ServiceProvider;
}

constexpr static int atlas_width{14};
namespace fornani::gui {
auto get_action_control_icon(automa::ServiceProvider& svc, config::DigitalAction action) -> sf::Sprite;
} // namespace gui
