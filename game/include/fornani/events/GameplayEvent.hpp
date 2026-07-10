
#pragma once

#include <fornani/core/Fwd.hpp>
#include <fornani/events/EventBase.hpp>
#include <string_view>

namespace fornani {

struct LaunchCutsceneEvent : Event<automa::ServiceProvider&, int> {};
struct PressPermanentSwitchEvent : Event<automa::ServiceProvider&, int> {};
struct SetCutsceneProgressionEvent : Event<int> {};
struct OpenVendorEvent : Event<automa::ServiceProvider&, int> {};
struct OpenBuilderEvent : Event<automa::ServiceProvider&, int> {};
struct AddMapMarkerEvent : Event<automa::ServiceProvider&, int, int, int> {};
struct PlaySongEvent : Event<int> {};
struct StartBattleEvent : Event<> {};
struct HealthRewardSequenceEvent : Event<automa::ServiceProvider&, player::Player&> {};
struct AbilityRewardSequenceEvent : Event<automa::ServiceProvider&, player::Player&, std::string_view> {};
struct TransitionEvent : Event<> {};
struct GiveBonusHealthEvent : Event<int> {};

} // namespace fornani
