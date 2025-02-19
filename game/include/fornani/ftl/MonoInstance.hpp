// Copyright (c) 2023-present Genesis Engine contributors (see LICENSE.txt)

#pragma once

#include <stdexcept>

namespace fornani::ftl
{
template <typename Type>
class MonoInstance
{
public:
	MonoInstance()
	{
		if (s_instance != nullptr) { throw std::runtime_error{"Instance already exists"}; }
		s_instance = static_cast<Type *>(this);
	}

	~MonoInstance() { s_instance = nullptr; }

	MonoInstance(MonoInstance const &)			   = delete;
	MonoInstance(MonoInstance &&)				   = delete;
	MonoInstance & operator=(MonoInstance const &) = delete;
	MonoInstance & operator=(MonoInstance &&)	   = delete;

	static Type & getInstance()
	{
		if (s_instance == nullptr) { throw std::runtime_error{"Instance does not exist"}; }
		return *s_instance;
	}

	static Type & self() { return getInstance(); }

	static bool exists() { return s_instance != nullptr; }

protected:
	inline static Type * s_instance{};
};
} // namespace fornani::ftl