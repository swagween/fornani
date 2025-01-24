#include "editor/util/Camera.hpp"

namespace pi {

Camera::Camera(sf::Vector2<float> dimensions) : bounding_box{sf::Rect<float>{{}, dimensions}} {}

} // namespace pi
