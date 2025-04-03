
#pragma once

namespace fornani {

class Polymorphic {
  public:
	Polymorphic() = default;
	Polymorphic(Polymorphic const&) = default;
	Polymorphic(Polymorphic&&) = default;
	virtual ~Polymorphic() = default;

	auto operator=(Polymorphic const&) -> Polymorphic& = default;
	auto operator=(Polymorphic&&) -> Polymorphic& = default;
};

class UniquePolymorphic {
  public:
	UniquePolymorphic(UniquePolymorphic const&) = delete;
	UniquePolymorphic(UniquePolymorphic&&) = delete;
	auto operator=(UniquePolymorphic const&) -> UniquePolymorphic& = delete;
	auto operator=(UniquePolymorphic&&) -> UniquePolymorphic& = delete;

	UniquePolymorphic() = default;
	virtual ~UniquePolymorphic() = default;
};

} // namespace fornani
