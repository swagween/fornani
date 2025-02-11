#pragma once

#include <filesystem>
#include <type_traits>

namespace fornani::util {

template <typename T>
concept UserType = std::is_class_v<T>;

template <typename T, typename... Ts>
concept ConstructibleUserType = UserType<T> && std::constructible_from<T, Ts...>;

template <typename Derived, typename Base>
concept Derives = std::derived_from<Derived, Base>;

} // namespace fornani::util