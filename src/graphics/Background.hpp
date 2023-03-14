//
//  Background.hpp
//
//  Created by Alex Frasca on 12/26/22.
//

#pragma once


#include <SFML/Graphics.hpp>
#include <array>


namespace bg {

static const size_t num_layers{5};

class Background {
    
public:
    
    Background() = default;
    Background(int lr, float spd);
    
    void update();
    void render(sf::RenderWindow& win);
    
    std::array<sf::Sprite, num_layers> sprites{};
    float scroll_speed{};
    int used_layers{};
    
    std::array<int, num_layers> frames{};
};

}


/* FLColor_hpp */
