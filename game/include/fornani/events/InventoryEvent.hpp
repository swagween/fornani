
#pragma once

#include <fornani/events/EventBase.hpp>
#include <string_view>

namespace fornani {

namespace automa {
struct ServiceProvider;
}

struct ReadItemByIDEvent : EventBase<int> {};
struct EquipItemByIDEvent : EventBase<automa::ServiceProvider&, int> {};
struct RevealItemByIDEvent : EventBase<int> {};
struct AcquireItemEvent : EventBase<automa::ServiceProvider&, std::string_view> {};
struct AcquireItemFromConsoleEvent : EventBase<automa::ServiceProvider&, int> {};
struct AcquireWeaponEvent : EventBase<automa::ServiceProvider&, std::string_view> {};
struct AcquireWeaponFromConsoleEvent : EventBase<automa::ServiceProvider&, int> {};
struct RemoveItemEvent : EventBase<automa::ServiceProvider&, std::string_view> {};
struct RemoveWeaponByIDEvent : EventBase<automa::ServiceProvider&, int> {};

} // namespace fornani
