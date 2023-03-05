//
//  EnumLookups.hpp
//
//  Created by Alex Frasca on 12/26/22.
//

#pragma once


#include <SFML/Graphics.hpp>
#include <unistd.h>
#include <stdio.h>
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

}

/* LookupTables_hpp */
