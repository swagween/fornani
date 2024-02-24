
#pragma once

namespace dir {

enum class LR { left, neutral, right };
enum class UND { up, neutral, down };

struct Direction {

	LR lr{};
	UND und{};

	void neutralize_und() {
		und = UND::neutral;
	}
	void neutralize_lr() {
		lr = LR::neutral;
	}

};

} // namespace util
