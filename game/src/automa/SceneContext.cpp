
#include <fornani/automa/SceneContext.hpp>
#include <fornani/service/ServiceProvider.hpp>

namespace fornani {

SceneContext::SceneContext(automa::ServiceProvider& svc) : transition(svc.window->f_screen_dimensions(), 96) {}

} // namespace fornani
