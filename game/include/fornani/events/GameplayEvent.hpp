
#pragma once

#include <fornani/events/EventBase.hpp>
#include <string_view>

namespace fornani {
namespace automa {
struct ServiceProvider;
}

struct LaunchCutsceneEvent : EventBase<automa::ServiceProvider&, int> {};
struct OpenVendorEvent : EventBase<automa::ServiceProvider&, int> {};
struct AddMapMarkerEvent : EventBase<automa::ServiceProvider&, int, int, int> {};
struct PlaySongEvent : EventBase<automa::ServiceProvider&, int> {};
struct StartBattleEvent : EventBase<> {};

} // namespace fornani
