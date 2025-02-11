// Copyright (c) 2023-present Genesis Engine contributors (see LICENSE.txt)

#pragma once

#include <stdexcept>

// If you have questions about this, ask Ian.

namespace fornani::ftl {

/**
 * @brief Template class enforcing unique instance ownership with singleton semantics.
 *
 * This class template implements a specialized singleton pattern that not only guarantees a single instance
 * exists but also enforces **exclusive ownership** of the instance. In other words, only one object of the
 * derived type can own and manage the associated static resource at any given time.
 *
 * ### Key Ownership Enforcement Features:
 * - **Exclusive Ownership Assignment:**
 *   The constructor verifies whether an instance already exists. If one does, it throws a runtime exception,
 *   ensuring that only one object can claim ownership.
 *
 * - **Controlled Resource Lifecycle:**
 *   The static member pointer (`s_instance`) is set during construction and reset in the destructor. This
 *   guarantees that the lifetime of the ownership is clearly defined.
 *
 * - **Prevention of Duplication:**
 *   Copy and move constructors and assignment operators are deleted, ensuring that the unique instance
 *   cannot be copied or moved, which would otherwise break the exclusive ownership contract.
 *
 * ### Usage Example:
 *
 * @code
 * #include "MonoInstance.hpp"
 * #include <iostream>
 *
 * // A class that exclusively owns its resource by deriving from MonoInstance.
 * class ResourceManager : public fornani::ftl::MonoInstance<ResourceManager> {
 * public:
 *     void manageResource() {
 *         // Exclusive resource management logic.
 *         std::cout << "Managing resource exclusively." << std::endl;
 *     }
 * };
 *
 * int main() {
 *     try {
 *         // Create the unique instance. Ownership is immediately enforced.
 *         ResourceManager resourceManager;
 *
 *         // Access the singleton instance using the static method.
 *         ResourceManager::getInstance().manageResource();
 *         // Alternatively:
 *         ResourceManager::self().manageResource();
 *
 *         // Check for instance existence.
 *         if (ResourceManager::exists()) {
 *             std::cout << "ResourceManager instance exists." << std::endl;
 *         }
 *     } catch (const std::runtime_error &error) {
 *         // Handle errors, such as multiple instantiation or accessing a non-existent instance.
 *         std::cerr << "Error: " << error.what() << std::endl;
 *         return -1;
 *     }
 *     return 0;
 * }
 * @endcode
 *
 * @tparam Type The type of the derived class. The derived class must inherit from MonoInstance<DerivedClass>.
 */
template <typename Type>
class MonoInstance {
  public:
	/**
	 * @brief Constructs the instance while enforcing exclusive ownership.
	 *
	 * The constructor checks whether an instance already exists. If it does, it throws a
	 * std::runtime_error, ensuring that no more than one instance can ever be created.
	 *
	 * @throws std::runtime_error if an instance already exists.
	 */
	MonoInstance() {
		if (s_instance != nullptr) { throw std::runtime_error{"Instance already exists"}; }
		s_instance = static_cast<Type*>(this);
	}

	/**
	 * @brief Destructor that relinquishes exclusive ownership.
	 *
	 * Upon destruction, the static instance pointer is reset to nullptr, releasing the ownership and
	 * allowing for a new instance to be created if needed.
	 */
	~MonoInstance() { s_instance = nullptr; }

	// Delete copy and move operations to prevent duplication of ownership.
	MonoInstance(MonoInstance const&) = delete;
	MonoInstance(MonoInstance&&) = delete;
	MonoInstance& operator=(MonoInstance const&) = delete;
	MonoInstance& operator=(MonoInstance&&) = delete;

	/**
	 * @brief Retrieves the unique instance that owns the resource.
	 *
	 * Provides global access to the exclusively owned instance. If no instance exists,
	 * a std::runtime_error is thrown.
	 *
	 * @return Reference to the singleton instance.
	 * @throws std::runtime_error if the instance does not exist.
	 */
	static Type& getInstance() {
		if (s_instance == nullptr) { throw std::runtime_error{"Instance does not exist"}; }
		return *s_instance;
	}

	/**
	 * @brief Alias for getInstance() for convenient access.
	 *
	 * @return Reference to the singleton instance.
	 * @throws std::runtime_error if the instance does not exist.
	 */
	static Type& self() { return getInstance(); }

	/**
	 * @brief Checks whether the unique instance exists.
	 *
	 * @return true if the instance exists, false otherwise.
	 */
	static bool exists() { return s_instance != nullptr; }

  protected:
	/**
	 * @brief Static pointer holding the unique instance.
	 *
	 * This pointer is set when the instance is constructed and reset upon destruction,
	 * ensuring exclusive ownership of the associated resource.
	 */
	inline static Type* s_instance{};
};
} // namespace fornani::ftl
