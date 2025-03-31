
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

} // namespace fornani
