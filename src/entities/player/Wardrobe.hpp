
#pragma once
#include "../../utils/BitFlags.hpp"
#include <unordered_map>
#include <string_view>

namespace automa {
struct ServiceProvider;
}

namespace gui {
class Console;
}

namespace player {

enum class ApparelType { shirt, pants, hairstyle, jacket, headgear, END };

class Wardrobe {
  public:
	Wardrobe();
	void equip(ApparelType type, int variant);
	void unequip(ApparelType type);
	int get_variant(ApparelType type);
  private:
	std::unordered_map<ApparelType, int> apparel{};
};

} // namespace player
