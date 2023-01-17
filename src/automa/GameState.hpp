//
//  GameState.hpp
//  automa
//
//  Created by Alex Frasca on 12/26/22.
//

#pragma once

#include <SFML/Graphics.hpp>
#include <cstdio>
#include <memory>
#include "../level/Map.hpp"
#include "../components/PhysicsComponent.hpp"
#include "../utils/Camera.hpp"
#include "../entities/player/Player.hpp"
#include "../setup/ServiceLocator.hpp"

namespace automa {

//auto camera = Camera();

//globals

enum class STATE {
    STATE_NULL,
    STATE_INIT,
    STATE_EXIT,
    STATE_MENU,
    STATE_OPTIONS,
    STATE_MAIN,
    STATE_DOJO
};

class GameState {
    
public:
    
    GameState() = default;
    GameState(int id) {
        
    }
    GameState& operator=(GameState&&) = delete;
    
    template<typename T> class StateID;
    
    virtual void init(const std::string& load_path) {};
    virtual void setTilesetTexture(std::vector<sf::Sprite>& tile_sprites) {};
    virtual void handle_events(sf::Event event) {
        
    };
    virtual void logic() {};
    virtual void render(sf::RenderWindow& win) {
        
        
    };
    
    STATE state = STATE::STATE_NULL;
};


}

namespace flstates {

// =======================================================================
//
//    MAIN_MENU
//
// =======================================================================

class MainMenu : public automa::GameState {
public:
    MainMenu() {
        state = automa::STATE::STATE_MENU;
    };
    void init(const std::string& load_path) {
    }
    void setTilesetTexture(std::vector<sf::Sprite>& tile_sprites) {
    }
    void handle_events(sf::Event event) {
        
        if (event.type == sf::Event::EventType::KeyPressed) {
        }
        
    }
    
    void logic() {}
    
    void render(sf::RenderWindow& win) {

    }
};

// =======================================================================
//
//    DOJO
//
// =======================================================================

class Dojo : public automa::GameState {
public:
    sf::Color FL_White          = sf::Color(235, 232, 249);
    sf::Color FL_Red            = sf::Color(236, 63,  95 );
    sf::Color FL_Fucshia        = sf::Color(215, 53,  180);
    sf::Color FL_Blue           = sf::Color(110, 98,  173);
    sf::Color FL_NavyBlue       = sf::Color(25,  35,  65 );
    sf::Color FL_DarkFucshia    = sf::Color(148, 40,  84 );
    sf::Color FL_Goldenrod      = sf::Color(247, 199, 74 );
    sf::Color FL_Orange         = sf::Color(226, 93,  11 );
    sf::Color FL_Black          = sf::Color(55,  49,  64 );
    sf::Color FL_Periwinkle     = sf::Color(159, 138, 247);
    sf::Color FL_Green          = sf::Color(81,  186, 155);
    Dojo() {
        state = automa::STATE::STATE_DOJO;
        svc::cameraLocator.get().set_position({0, 0});
        svc::playerLocator.get().set_position({400, 300});
    };
    void init(const std::string& load_path) {
        map.load(load_path);
    }
    void setTilesetTexture(std::vector<sf::Sprite>& tile_sprites) {
        tileset = svc::assetLocator.get().sp_tileset;
    }
    void handle_events(sf::Event event) {
        svc::playerLocator.get().handle_events(event);
        if (event.type == sf::Event::EventType::KeyPressed) {
            if (event.key.code == sf::Keyboard::Left) {
            }
            if (event.key.code == sf::Keyboard::Right) {
            }
        }
    }
    
    void logic() {
        map.update();
        svc::cameraLocator.get().center(svc::playerLocator.get().physics.position);
        svc::cameraLocator.get().update();
        svc::playerLocator.get().update();
    }
    
    void render(sf::RenderWindow& win) {
        for(int i = 0; i < map.layers.size(); ++i) {
            for(int j = 0; j < map.layers.at(i).grid.cells.size(); ++j) {
                if(map.layers.at(i).collidable) {
                    if(map.layers.at(i).grid.cells.at(j).value > 0) {
                        tileset.at(map.layers.at(i).grid.cells.at(j).value).setPosition(map.layers.at(i).grid.cells.at(j).bounding_box.shape_x - svc::cameraLocator.get().physics.position.x, map.layers.at(i).grid.cells.at(j).bounding_box.shape_y - svc::cameraLocator.get().physics.position.y);
                        win.draw(tileset.at(map.layers.at(i).grid.cells.at(j).value));
                        sf::RectangleShape box{};
                        if(map.layers.at(i).grid.cells.at(j).collision_check) {
                            box.setFillColor(FL_Periwinkle);
                            box.setOutlineColor(FL_White);
                        } else {
                            box.setFillColor(FL_DarkFucshia);
                            box.setOutlineColor(FL_Red);
                        }
                        
                        box.setOutlineThickness(-1);
                        box.setPosition(map.layers.at(i).grid.cells.at(j).bounding_box.shape_x - svc::cameraLocator.get().physics.position.x, map.layers.at(i).grid.cells.at(j).bounding_box.shape_y - svc::cameraLocator.get().physics.position.y);
                        box.setSize({32, 32});
                        win.draw(box);
                    }
                }
                
            }
        }
        sf::Vector2<float> player_pos = svc::playerLocator.get().physics.position - svc::cameraLocator.get().physics.position;
//        svc::assetLocator.get().s_nani_idle.setPosition(player_pos);
        sf::RectangleShape plr{};
        plr.setPosition(player_pos.x, player_pos.y);
        plr.setFillColor(FL_Goldenrod);
        plr.setOutlineColor(FL_White);
        plr.setOutlineThickness(-1);
        plr.setSize({PLAYER_WIDTH, PLAYER_HEIGHT});
        
        win.draw(plr);
//        win.draw(svc::assetLocator.get().s_nani_idle);
    }
    
    world::Map map{};
    std::vector<sf::Sprite> tileset{};
    
};

}

/* GameState_hpp */
