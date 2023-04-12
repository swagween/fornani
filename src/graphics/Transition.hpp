//
//  Transition.hpp
//
//  Created by Alex Frasca on 12/26/22.
//

#pragma once


#include <SFML/Graphics.hpp>
#include "../setup/ServiceLocator.hpp"
#include <array>


namespace flfx {

class Transition {
    
public:

    Transition(int d) : duration(d) {
        box.setPosition(0, 0);
        box.setSize(sf::Vector2<float>(cam::screen_dimensions.x, cam::screen_dimensions.y));
        current_frame = 0;
    }
    
    void update();
    void render(sf::RenderWindow& win);

    const int duration{};
    int current_frame{};
    uint8_t alpha{255};
    bool done{ false };
    bool fade_out{ false };
    bool fade_in{ false };
    int rate{ 4 };

    sf::RectangleShape box{};

};

}


/* Transition_hpp */
