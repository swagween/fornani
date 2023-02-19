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
#include <chrono>
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
    
    using Time = std::chrono::duration<float>;
    
    GameState() = default;
    GameState(int id) {
        
    }
    GameState& operator=(GameState&&) = delete;
    
    template<typename T> class StateID;
    
    virtual void init(const std::string& load_path) {};
    virtual void setTilesetTexture(std::vector<sf::Sprite>& tile_sprites) {};
    virtual void handle_events(sf::Event event) {
        
    };
    virtual void logic(Time dt) {};
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
    
    void logic(Time dt) {}
    
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
        svc::playerLocator.get().behavior.current_state = std::move(std::make_unique<behavior::Behavior>(behavior::idle));
    }
    void setTilesetTexture(std::vector<sf::Sprite>& tile_sprites) {
        tileset = svc::assetLocator.get().sp_tileset_provisional;
    }
    void handle_events(sf::Event event) {
        svc::playerLocator.get().handle_events(event);
        if (event.type == sf::Event::EventType::KeyPressed) {
            if (event.key.code == sf::Keyboard::H) {
                show_colliders = !show_colliders;
            }
        }
    }
    
    void logic(Time dt) {
        map.update();
        svc::cameraLocator.get().center(svc::playerLocator.get().anchor_point);
        svc::cameraLocator.get().update();
        svc::playerLocator.get().update(dt);
    }
    
    void render(sf::RenderWindow& win) {
        for(int i = 0; i < map.layers.size(); ++i) {
            for(int j = 0; j < map.layers.at(i).grid.cells.size(); ++j) {
                if(map.layers.at(i).collidable) {
                    if(map.layers.at(i).grid.cells.at(j).value > 0) {
                        tileset.at(map.layers.at(i).grid.cells.at(j).value).setPosition(map.layers.at(i).grid.cells.at(j).bounding_box.shape_x - svc::cameraLocator.get().physics.position.x, map.layers.at(i).grid.cells.at(j).bounding_box.shape_y - svc::cameraLocator.get().physics.position.y);
                        win.draw(tileset.at(map.layers.at(i).grid.cells.at(j).value));
//
                        if(map.layers.at(i).grid.cells.at(j).collision_check) {
                            sf::RectangleShape box{};
                            box.setFillColor(sf::Color(255, 255, 255, 50));
                            box.setOutlineColor(FL_White);
                            box.setOutlineThickness(-1);
                            box.setPosition(map.layers.at(i).grid.cells.at(j).bounding_box.shape_x - svc::cameraLocator.get().physics.position.x, map.layers.at(i).grid.cells.at(j).bounding_box.shape_y - svc::cameraLocator.get().physics.position.y);
                            box.setSize({32, 32});
//                            win.draw(box);
                        }
                    }
                }
            }
        }
        
        if(show_colliders) {
            sf::Vector2<float> jumpbox_pos = sf::operator-(svc::playerLocator.get().jumpbox.vertices.at(0), svc::cameraLocator.get().physics.position);
            sf::RectangleShape jbx{};
            jbx.setPosition(jumpbox_pos.x, jumpbox_pos.y);
            jbx.setFillColor(sf::Color(110, 98,  173, 40));
            jbx.setOutlineColor(sf::Color(235, 232, 249, 80));
            jbx.setOutlineThickness(-1);
            jbx.setSize({(float)svc::playerLocator.get().jumpbox.shape_w, (float)svc::playerLocator.get().jumpbox.shape_h});
            win.draw(jbx);
            
            sf::Vector2<float> leftbox_pos = sf::operator-(svc::playerLocator.get().left_detector.vertices.at(0), svc::cameraLocator.get().physics.position);
            sf::RectangleShape leftbox{};
            leftbox.setPosition(leftbox_pos.x, leftbox_pos.y);
            leftbox.setFillColor(sf::Color(110, 98,  173, 40));
            leftbox.setOutlineColor(sf::Color(235, 232, 249, 80));
            leftbox.setOutlineThickness(-1);
            leftbox.setSize({(float)svc::playerLocator.get().left_detector.shape_w, (float)svc::playerLocator.get().left_detector.shape_h});
            win.draw(leftbox);
            
            sf::Vector2<float> rightbox_pos = sf::operator-(svc::playerLocator.get().right_detector.vertices.at(0), svc::cameraLocator.get().physics.position);
            sf::RectangleShape rightbox{};
            rightbox.setPosition(rightbox_pos.x, rightbox_pos.y);
            rightbox.setFillColor(sf::Color(110, 98,  173, 40));
            rightbox.setOutlineColor(sf::Color(235, 232, 249, 80));
            rightbox.setOutlineThickness(-1);
            rightbox.setSize({(float)svc::playerLocator.get().right_detector.shape_w, (float)svc::playerLocator.get().right_detector.shape_h});
            win.draw(rightbox);
            
            sf::Vector2<float> predictive_hurtbox_pos = sf::operator-(svc::playerLocator.get().predictive_hurtbox.vertices.at(0), svc::cameraLocator.get().physics.position);
            sf::RectangleShape predictive_hbx{};
            predictive_hbx.setPosition(predictive_hurtbox_pos.x, predictive_hurtbox_pos.y);
            predictive_hbx.setFillColor(sf::Color(215, 53,  180, 10));
            predictive_hbx.setOutlineColor(FL_Fucshia);
            predictive_hbx.setOutlineThickness(-1);
            predictive_hbx.setSize({(float)svc::playerLocator.get().predictive_hurtbox.shape_w, (float)svc::playerLocator.get().predictive_hurtbox.shape_h});
            win.draw(predictive_hbx);
            
            sf::Vector2<float> hurtbox_pos = sf::operator-(svc::playerLocator.get().hurtbox.vertices.at(0), svc::cameraLocator.get().physics.position);
            sf::RectangleShape hbx{};
            hbx.setPosition(hurtbox_pos.x, hurtbox_pos.y);
            hbx.setFillColor(sf::Color(159, 138, 247, 200));
            hbx.setOutlineColor(FL_White);
            hbx.setOutlineThickness(-1);
            hbx.setSize({(float)svc::playerLocator.get().hurtbox.shape_w, (float)svc::playerLocator.get().hurtbox.shape_h});
            win.draw(hbx);
        }
        
        sf::Vector2<float> player_pos = svc::playerLocator.get().physics.position - svc::cameraLocator.get().physics.position;
        svc::playerLocator.get().current_sprite = svc::assetLocator.get().sp_nani.at(svc::playerLocator.get().behavior.current_state->params.lookup_value + svc::playerLocator.get().behavior.current_state->params.current_frame);
        svc::playerLocator.get().current_sprite.setPosition(player_pos.x - (48 - PLAYER_WIDTH)/2, player_pos.y - (48 - PLAYER_HEIGHT));
        win.draw(svc::playerLocator.get().current_sprite);
        
        svc::assetLocator.get().sp_hud.setPosition(20, 20);
        svc::assetLocator.get().sp_hud2x.setPosition(20, 20);
        win.draw(svc::assetLocator.get().sp_hud);
//        win.draw(svc::assetLocator.get().s_hud2x);
        
//        svc::playerLocator.get().update_animation();
        
        
    }
    
    world::Map map{};
    std::vector<sf::Sprite> tileset{};
    bool show_colliders = false;
    
};

}

/* GameState_hpp */
