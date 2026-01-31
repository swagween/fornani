
#pragma once

#include <fornani/events/EventBase.hpp>
#include <string_view>

namespace fornani {

namespace automa {
struct ServiceProvider;
}

struct LoadFileEvent : EventBase<std::string_view, std::string_view> {};
struct NewFileEvent : EventBase<int> {};
struct ReloadSaveEvent : EventBase<automa::ServiceProvider&, int> {};
struct ReturnToMainMenuEvent : EventBase<> {};

} // namespace fornani
