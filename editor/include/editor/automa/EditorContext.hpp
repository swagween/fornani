
#pragma once

#include <SFML/Graphics.hpp>
#include <fornani/utils/BitFlags.hpp>

namespace pi {

enum class EditorContextFlags { new_room };

struct EditorContext {
	fornani::util::BitFlags<EditorContextFlags> flags{};
	sf::Vector2i metagrid_position{};
};

} // namespace pi
