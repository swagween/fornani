
#pragma once

#include <fornani/events/EventBase.hpp>
#include <string_view>

namespace fornani {

namespace automa {
struct ServiceProvider;
}

struct LoadFileEvent : Event<std::string_view, std::string_view> {};
struct NewFileEvent : Event<int> {};
struct ReloadSaveEvent : Event<automa::ServiceProvider&, int> {};
struct ReturnToMainMenuEvent : Event<> {};

} // namespace fornani
