
#pragma once

namespace dir {

	// the exclusion of "N" in the class name is purposeful; there is no "neutral" direction between L and R.
	// the neutral state is used for setting the und state in specific use cases, like weapon direction.
enum class LR { left, neutral, right };
enum class UND { up, neutral, down };

struct Direction {

	LR lr{LR::neutral};
	UND und{UND::neutral};

	void neutralize_und() {
		und = UND::neutral;
	}
	void neutralize_lr() {
		lr = LR::neutral;
	}
	float as_float() { return lr == LR::left ? -1.f : (lr == LR::right ? 1.f : 0.f); }

	std::string print_und() const { return "UND: " + (std::string)(und == UND::up ? "UP" : (und == UND::neutral ? "NEUTRAL" : "DOWN")); }
	std::string print_lr() const { return "LR: " + (std::string)(lr == LR::left ? "LEFT" : (lr == LR::neutral ? "NEUTRAL" : "RIGHT")); }

};

} // namespace util
