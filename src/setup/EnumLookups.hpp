
#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include <unordered_map>

namespace lookup {


inline const float SPACING{ 32.0f };
inline const int NUM_STYLES{21};

const inline float unit_size_f{ 32.0f };
inline int const unit_size_i{32};

inline float const min_hook_length{32.f};
inline float const max_hook_length{256.f};

enum class CONTROLS {
    JUMP,
    SHOOT,
    DASH,
    MOVE_LEFT,
	MOVE_RIGHT,
	LOOK_UP,
	LOOK_DOWN,
    ARMS_SWITCH_LEFT,
    ARMS_SWITCH_RIGHT,
    INSPECT,
    SELECT,
    PAUSE
};

enum class STYLE {
    FIRSTWIND,
    OVERTURNED,
    GRUB,
    TOXIC,
    BASE,
    FROZEN,
    NIGHT,
    WORM,
    SKY,
    ASH,
    GREATWING,
    ICE,
    SNOW,
    STONE,
    ABANDONED,
    ANCIENT,
    FACTORY,
    SHADOW,
    HOARDER,
    MANSION,
    PROVISIONAL
};

enum class TILE_TYPE {
    TILE_BASIC,
    TILE_GROUND_RAMP,
    TILE_CEILING_RAMP,
    TILE_MOVEABLE,
    TILE_PLATFORM,
    TILE_DEATH_SPIKES,
    TILE_BREAKABLE,
    TILE_LADDER,
    TILE_SPIKES,
    TILE_ICY
};

enum class ALPHABET {
    A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
    ONE, TWO, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, NINE, ZERO, APOSTROPHE, PERIOD,
    EXCLAM, PERCENT, OPEN_ANGLED_BRACKET, CLOSED_ANGLED_BRACKET, QUESTION_MARK, COLON,
    DASH, UNDESRCORE, HYPHEN, OPEN_PARENTHESIS, CLOSED_PARENTHESIS, HASH, PLUS, SQUARE
};

inline std::unordered_map<int, STYLE> get_style {
    {0,     STYLE::FIRSTWIND    },
    {1,     STYLE::OVERTURNED   },
    {2,     STYLE::GRUB         },
    {3,     STYLE::TOXIC        },
    {4,     STYLE::BASE         },
    {5,     STYLE::FROZEN       },
    {6,     STYLE::NIGHT        },
    {7,     STYLE::WORM         },
    {8,     STYLE::SKY          },
    {9,     STYLE::ASH          },
    {10,    STYLE::GREATWING    },
    {11,    STYLE::ICE          },
    {12,    STYLE::SNOW         },
    {13,    STYLE::STONE        },
    {14,    STYLE::ABANDONED    },
    {15,    STYLE::ANCIENT      },
    {16,    STYLE::FACTORY      },
    {17,    STYLE::SHADOW       },
    {18,    STYLE::HOARDER      },
    {19,    STYLE::MANSION      },
    {20,    STYLE::PROVISIONAL  }
};

inline std::unordered_map<STYLE, int> get_style_id {
    {STYLE::FIRSTWIND, 0    },
    {STYLE::OVERTURNED, 1   },
    {STYLE::GRUB, 2         },
    {STYLE::TOXIC, 3        },
    {STYLE::BASE, 4         },
    {STYLE::FROZEN, 5       },
    {STYLE::NIGHT, 6        },
    {STYLE::WORM, 7         },
    {STYLE::SKY, 8          },
    {STYLE::ASH, 9          },
    {STYLE::GREATWING, 10   },
    {STYLE::ICE, 11         },
    {STYLE::SNOW, 12        },
    {STYLE::STONE, 13       },
    {STYLE::ABANDONED, 14   },
    {STYLE::ANCIENT, 15     },
    {STYLE::FACTORY, 16     },
    {STYLE::SHADOW, 17      },
    {STYLE::HOARDER, 18     },
    {STYLE::MANSION, 19     },
    {STYLE::PROVISIONAL, 20 }
};

inline std::unordered_map<STYLE, const char*> get_style_string {
    {STYLE::FIRSTWIND,  "firstwind"     },
    {STYLE::OVERTURNED, "overturned"    },
    {STYLE::GRUB,       "grub"          },
    {STYLE::TOXIC,      "toxic"         },
    {STYLE::BASE,       "base"          },
    {STYLE::FROZEN,     "frozen"        },
    {STYLE::NIGHT,      "night"         },
    {STYLE::WORM,       "worm"          },
    {STYLE::SKY,        "sky"           },
    {STYLE::ASH,        "ash"           },
    {STYLE::GREATWING,  "greatwing"     },
    {STYLE::ICE,        "ice"           },
    {STYLE::SNOW,       "snow"          },
    {STYLE::STONE,      "stone"         },
    {STYLE::ABANDONED,  "abandoned"     },
    {STYLE::ANCIENT,    "ancient"       },
    {STYLE::FACTORY,    "factory"       },
    {STYLE::SHADOW,     "shadow"        },
    {STYLE::HOARDER,    "hoarder"       },
    {STYLE::MANSION,    "mansion"       },
    {STYLE::PROVISIONAL,"provisional"   }
};

inline std::unordered_map<char, int> get_character {
    {'a', 0},
    {'b', 1},
    {'c', 2},
    {'d', 3},
    {'e', 4},
    {'f', 5},
    {'g', 6},
    {'h', 7},
    {'i', 8},
    {'j', 9},
    {'k', 10},
    {'l', 11},
    {'m', 12},
    {'n', 13},
    {'o', 14},
    {'p', 15},
    {'q', 16},
    {'r', 17},
    {'s', 18},
    {'t', 19},
    {'u', 20},
    {'v', 21},
    {'w', 22},
    {'x', 23},
    {'y', 24},
    {'z', 25},
    {'1', 26},
    {'2', 27},
    {'3', 28},
    {'4', 29},
    {'5', 30},
    {'6', 31},
    {'7', 32},
    {'8', 33},
    {'9', 34},
    {'0', 35},
    {'\'',36},
    {'.', 37},
    {'!', 38},
    {'%', 39},
    {'<', 40},
    {'>', 41},
    {'?', 42},
    {':', 43},
    // {'�', 44},
    {'_', 45},
    {'-', 46},
    {'(', 47},
    {')', 48},
    {'#', 49},
    {'+', 50},
    {'`', 51}
};

inline std::unordered_map<char, int> get_orb_number{
    {'0', 0},
    {'1', 1},
    {'2', 2},
    {'3', 3},
    {'4', 4},
    {'5', 5},
    {'6', 6},
    {'7', 7},
    {'8', 8},
    {'9', 9}
};


inline std::unordered_map<CONTROLS, sf::Keyboard::Key> control_to_key {
    {CONTROLS::JUMP, sf::Keyboard::Z},
    {CONTROLS::SHOOT, sf::Keyboard::X},
    {CONTROLS::DASH, sf::Keyboard::Z},
	{CONTROLS::MOVE_LEFT, sf::Keyboard::Left},
	{CONTROLS::MOVE_RIGHT, sf::Keyboard::Right},
	{CONTROLS::LOOK_UP, sf::Keyboard::Up},
	{CONTROLS::LOOK_DOWN, sf::Keyboard::Down},
    {CONTROLS::ARMS_SWITCH_LEFT, sf::Keyboard::A},
	{CONTROLS::ARMS_SWITCH_RIGHT, sf::Keyboard::S},
	{CONTROLS::INSPECT, sf::Keyboard::Down},
	{CONTROLS::SELECT, sf::Keyboard::Z},
    {CONTROLS::PAUSE, sf::Keyboard::Q}

};

inline std::unordered_map<int, TILE_TYPE> tile_lookup{};

inline void populate_lookup() {
	for (int i = 0; i < 256; ++i) {
		if (i < 192 && i >= 0)   { tile_lookup.insert({ i, TILE_TYPE::TILE_BASIC });        }
        if (i < 208 && i >= 192) { tile_lookup.insert({ i, TILE_TYPE::TILE_CEILING_RAMP }); }
		if (i < 224 && i >= 208) { tile_lookup.insert({ i, TILE_TYPE::TILE_GROUND_RAMP });         }
		if (i < 228 && i >= 224) { tile_lookup.insert({ i, TILE_TYPE::TILE_BASIC });        }
		if (i < 232 && i >= 228) { tile_lookup.insert({ i, TILE_TYPE::TILE_BASIC });        }
		if (i < 236 && i >= 232) { tile_lookup.insert({ i, TILE_TYPE::TILE_BASIC });        }
		if (i < 240 && i >= 236) { tile_lookup.insert({ i, TILE_TYPE::TILE_PLATFORM });     }
		if (i < 244 && i >= 240) { tile_lookup.insert({ i, TILE_TYPE::TILE_DEATH_SPIKES });        }
		if (i < 248 && i >= 244) { tile_lookup.insert({ i, TILE_TYPE::TILE_BREAKABLE });    }
		if (i < 252 && i >= 248) { tile_lookup.insert({ i, TILE_TYPE::TILE_LADDER });       }
		if (i < 256 && i >= 252) { tile_lookup.insert({ i, TILE_TYPE::TILE_SPIKES });       }
	}
}

}
