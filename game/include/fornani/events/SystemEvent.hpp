
#pragma once

#include <fornani/events/EventBase.hpp>
#include <string_view>

namespace fornani {
struct AppContext;
namespace automa {
struct ServiceProvider;
}

struct LoadFileEvent : Event<std::string_view, std::string_view> {};
struct NewFileEvent : Event<int> {};
struct ReloadSaveEvent : Event<automa::ServiceProvider&, int> {};
struct ReturnToMainMenuEvent : Event<> {};
struct PauseEvent : Event<automa::ServiceProvider&> {};
struct SetLangaugeEvent : Event<AppContext&, std::string_view> {};

} // namespace fornani
