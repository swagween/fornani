//
//  EnumLookups.hpp
//
//  Created by Alex Frasca on 12/26/22.
//

#pragma once


#include <SFML/Graphics.hpp>
#include <string>
#include <unordered_map>

namespace lookup {

inline const int NUM_STYLES{20};

enum class BACKDROP {
    BG_NULL,
    BG_SUNSET,
    BG_SUNRISE,
    BG_OPEN_SKY,
    BG_RED_SKY,
    BG_BLACK
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
    MANSION
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
    {19,    STYLE::MANSION      }
};

inline std::unordered_map<int, BACKDROP> get_backdrop {
    {-1,    BACKDROP::BG_NULL       },
    {0,     BACKDROP::BG_SUNSET     },
    {1,     BACKDROP::BG_SUNRISE    },
    {2,     BACKDROP::BG_OPEN_SKY   },
    {3,     BACKDROP::BG_RED_SKY    },
    {4,     BACKDROP::BG_BLACK      }
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
    {STYLE::MANSION, 19     }
};

inline std::unordered_map<BACKDROP, int> get_backdrop_id {
    {BACKDROP::BG_NULL, -1      },
    {BACKDROP::BG_SUNSET, 0     },
    {BACKDROP::BG_SUNRISE, 1    },
    {BACKDROP::BG_OPEN_SKY, 2   },
    {BACKDROP::BG_RED_SKY, 3    },
    {BACKDROP::BG_BLACK, 4      }
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
    {STYLE::MANSION,    "mansion"       }
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
    {'\'', 36}
};

}

/* LookupTables_hpp */
