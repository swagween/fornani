
#pragma once

#include <fornani/events/EventBase.hpp>
#include <string_view>

namespace fornani {

namespace automa {
struct ServiceProvider;
}

struct ReadItemByIDEvent : Event<int> {};
struct EquipItemByIDEvent : Event<automa::ServiceProvider&, int> {};
struct RevealItemByIDEvent : Event<int> {};
struct AcquireItemEvent : Event<automa::ServiceProvider&, std::string_view> {};
struct AcquireItemFromConsoleEvent : Event<automa::ServiceProvider&, int> {};
struct AcquireWeaponEvent : Event<automa::ServiceProvider&, std::string_view> {};
struct AcquireWeaponFromConsoleEvent : Event<automa::ServiceProvider&, int> {};
struct RemoveItemEvent : Event<automa::ServiceProvider&, std::string_view> {};
struct RemoveWeaponByIDEvent : Event<automa::ServiceProvider&, int> {};
struct RemoveItemByIDEvent : Event<automa::ServiceProvider&, int> {};

} // namespace fornani
