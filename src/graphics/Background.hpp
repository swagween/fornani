//
//  Background.hpp
//
//  Created by Alex Frasca on 12/26/22.
//

#pragma once


#include <SFML/Graphics.hpp>
#include <array>
#include <unordered_map>
#include "../setup/ServiceLocator.hpp"


namespace bg {

static const size_t num_layers{5};

enum class BACKDROP {
    BG_NULL,
    BG_DUSK,
    BG_SUNRISE,
    BG_OPEN_SKY,
    BG_ROSY_HAZE,
    BG_NIGHT,
    BG_DAWN,
    BG_OVERCAST
};

inline std::unordered_map<int, sf::Texture&> get_backdrop_texture {
    {0,     svc::assetLocator.get().t_bg_dusk       },
    {1,     svc::assetLocator.get().t_bg_sunrise    },
    {2,     svc::assetLocator.get().t_bg_opensky    },
    {3,     svc::assetLocator.get().t_bg_rosyhaze   },
    {4,     svc::assetLocator.get().t_bg_dawn       },
    {5,     svc::assetLocator.get().t_bg_night      },
    {6,     svc::assetLocator.get().t_bg_overcast   }
};

class Background {
    
public:
    
    Background() = default;
    Background(int lr, float spd, int bg_id);
    
    void update();
    void render(sf::RenderWindow& win);
    
    std::array<sf::Sprite, num_layers> sprites{};
    float scroll_speed{};
    int used_layers{};
    
    std::array<int, num_layers> frames{};
};

}


/* FLColor_hpp */
