
#pragma once

#include <fornani/events/EventBase.hpp>
#include <ksignal/ksignal.hpp>
#include <string_view>

namespace fornani {

namespace automa {
struct ServiceProvider;
}

struct NPCPiggybackEvent : Event<automa::ServiceProvider&, int> {};
struct NPCPopConversationEvent : Event<> {};
struct NPCVoiceCueEvent : Event<automa::ServiceProvider&, int> {};

} // namespace fornani
