
#pragma once

#include <fornani/events/EventBase.hpp>
#include <string_view>

namespace fornani {
namespace automa {
struct ServiceProvider;
}

struct LaunchCutsceneEvent : Event<automa::ServiceProvider&, int> {};
struct OpenVendorEvent : Event<automa::ServiceProvider&, int> {};
struct AddMapMarkerEvent : Event<automa::ServiceProvider&, int, int, int> {};
struct PlaySongEvent : Event<int> {};
struct StartBattleEvent : Event<> {};

} // namespace fornani
