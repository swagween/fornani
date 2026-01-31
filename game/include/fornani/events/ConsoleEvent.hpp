
#pragma once

#include <fornani/events/EventBase.hpp>
#include <string_view>

namespace fornani {

namespace automa {
struct ServiceProvider;
}

struct NPCPiggybackEvent : EventBase<automa::ServiceProvider&, int> {};
struct NPCPopConversationEvent : EventBase<> {};
struct NPCVoiceCueEvent : EventBase<automa::ServiceProvider&, int> {};

} // namespace fornani
