#pragma once
#include <SFML/Graphics.hpp>
#include "../setup/ControllerMap.hpp"

namespace automa {
struct ServiceProvider;
}

namespace gui {

auto get_action_control_icon(automa::ServiceProvider& svc, config::DigitalAction action) -> sf::Sprite;

} // namespace gui
