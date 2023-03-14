//
//  Background.cpp
//  
//
//  Created by Alex Frasca on 12/26/22.
//

#include "Background.hpp"
#include "../setup/ServiceLocator.hpp"

namespace bg {

Background::Background(int lr, float spd) : used_layers(lr), scroll_speed(spd) {
    int idx = 0;
    for(auto& sprite : sprites) {
        sprite.setTextureRect(sf::IntRect({0, 540 * idx}, {3840, 540}));
        sprite.setTexture(svc::assetLocator.get().t_bg_rosyhaze);
        ++idx;
    }
}

void Background::update() {
    int idx = 0;
    for(auto& sprite : sprites) {
        int offset = idx * (-frames[idx] * scroll_speed);
        if(offset < -1920) {frames[idx] = 0; }
        sprite.setPosition(idx * (-frames[idx] * scroll_speed), 0);
        ++idx;
    }
    for(auto& frame : frames) {
        ++frame;
    }
}

void Background::render(sf::RenderWindow &win) {
    for(auto& sprite : sprites) {
        win.draw(sprite);
    }
}

}
