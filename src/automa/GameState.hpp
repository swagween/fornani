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
#include "../setup/LookupTables.hpp"
#include "../gui/HUD.hpp"
#include "../graphics/Background.hpp"

namespace automa {

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
    virtual void setTilesetTexture(sf::Texture& t) {};
    virtual void handle_events(sf::Event& event) {
        
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
    void setTilesetTexture(sf::Texture& t) {
    }
    void handle_events(sf::Event& event) {
        
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
    
    Dojo() {
        state = automa::STATE::STATE_DOJO;
        svc::cameraLocator.get().set_position({1, 1});
        svc::playerLocator.get().set_position({360, 500});
    }
    void init(const std::string& load_path) {
        map.load(load_path);
        svc::playerLocator.get().behavior.current_state = behavior::Behavior(behavior::idle);
        tileset = svc::assetLocator.get().tilesets.at(lookup::get_style_id.at(map.style));
        for(int i = 0; i < 16; ++i) {
            for(int j = 0; j < 16; ++j) {
                tileset_sprites.push_back(sf::Sprite());
                tileset_sprites.back().setTexture(tileset);
                tileset_sprites.back().setTextureRect(sf::IntRect({j * TILE_WIDTH, i * TILE_WIDTH}, {TILE_WIDTH, TILE_WIDTH}));
                
                
                svc::assetLocator.get().sp_bryn_test.setTextureRect(sf::IntRect({0, 0}, {128, 256}));
                svc::assetLocator.get().sp_ui_test.setTextureRect(sf::IntRect({0, 0}, {420, 128}));
            }
        }
        svc::playerLocator.get().collider.physics.zero();
        for(auto& portal : map.portals) {
            if(portal.destination_map_id == svc::stateControllerLocator.get().source_id) {
                sf::Vector2<float> spawn_position{ portal.position.x + std::floor(portal.dimensions.x / 2), portal.position.y + portal.dimensions.y - PLAYER_HEIGHT};
                svc::playerLocator.get().set_position(spawn_position);
                svc::cameraLocator.get().center(spawn_position);
            }
        }

//        svc::assetLocator.get().abandoned.setVolume(50);
//        svc::assetLocator.get().abandoned.play();
//        svc::assetLocator.get().abandoned.setLoop(true);
        /*
        svc::assetLocator.get().three_pipes.setVolume(svc::assetLocator.get().music_vol);
        svc::assetLocator.get().three_pipes.play();
        svc::assetLocator.get().three_pipes.setLoop(true);
        */
//        svc::assetLocator.get().brown_noise.setVolume(20);
//        svc::assetLocator.get().brown_noise.play();
//        svc::assetLocator.get().brown_noise.setLoop(true);
        
    }
    void handle_events(sf::Event& event) {
        if (!svc::playerLocator.get().input_flags.restricted) {
            svc::playerLocator.get().handle_events(event);
        }
        if (event.type == sf::Event::EventType::KeyPressed) {
            if (event.key.code == sf::Keyboard::H) {
                show_colliders = !show_colliders;
            }
        }
        
        if (event.type == sf::Event::EventType::KeyPressed) {
            if (event.key.code == sf::Keyboard::B) {
                x++;
                if(x % 4 == 0) {x = 0;}
                svc::assetLocator.get().sp_bryn_test.setTextureRect(sf::IntRect({x*128, 0}, {128, 256}));
            }
        }
    }
    
    void logic() {
        map.update();
        hud.update();
        svc::cameraLocator.get().center(svc::playerLocator.get().anchor_point);
        svc::cameraLocator.get().update();
        svc::cameraLocator.get().restrict_movement(map.real_dimensions);
        svc::playerLocator.get().update(svc::clockLocator.get().elapsed_time);
        svc::assetLocator.get().three_pipes.setVolume(svc::assetLocator.get().music_vol);
    }
    
    void render(sf::RenderWindow& win) {
        
        map.render_background(win, tileset_sprites, svc::cameraLocator.get().physics.position);
        
        if(show_colliders) {
            sf::Vector2<float> jumpbox_pos = sf::operator-(svc::playerLocator.get().collider.jumpbox.vertices.at(0), svc::cameraLocator.get().physics.position);
            sf::RectangleShape jbx{};
            jbx.setPosition(jumpbox_pos.x, jumpbox_pos.y);
            jbx.setFillColor(sf::Color::Transparent);
            jbx.setOutlineColor(sf::Color(235, 232, 249, 80));
            jbx.setOutlineThickness(-1);
            jbx.setSize({(float)svc::playerLocator.get().collider.jumpbox.shape_w, (float)svc::playerLocator.get().collider.jumpbox.shape_h});
            win.draw(jbx);
            
            sf::Vector2<float> leftbox_pos = sf::operator-(svc::playerLocator.get().collider.left_detector.vertices.at(0), svc::cameraLocator.get().physics.position);
            sf::RectangleShape leftbox{};
            leftbox.setPosition(leftbox_pos.x, leftbox_pos.y);
            leftbox.setFillColor(sf::Color::Transparent);
            leftbox.setOutlineColor(sf::Color(235, 232, 249, 80));
            leftbox.setOutlineThickness(-1);
            leftbox.setSize({(float)svc::playerLocator.get().collider.left_detector.shape_w, (float)svc::playerLocator.get().collider.left_detector.shape_h});
            win.draw(leftbox);
            
            sf::Vector2<float> rightbox_pos = sf::operator-(svc::playerLocator.get().collider.right_detector.vertices.at(0), svc::cameraLocator.get().physics.position);
            sf::RectangleShape rightbox{};
            rightbox.setPosition(rightbox_pos.x, rightbox_pos.y);
            rightbox.setFillColor(sf::Color::Transparent);
            rightbox.setOutlineColor(sf::Color(235, 232, 249, 80));
            rightbox.setOutlineThickness(-1);
            rightbox.setSize({(float)svc::playerLocator.get().collider.right_detector.shape_w, (float)svc::playerLocator.get().collider.right_detector.shape_h});
            win.draw(rightbox);
            
            sf::Vector2<float> hbx_pos = sf::operator-(svc::playerLocator.get().collider.bounding_box.vertices.at(0), svc::cameraLocator.get().physics.position);
            sf::RectangleShape hbx{};
            hbx.setPosition(hbx_pos.x, hbx_pos.y);
            hbx.setFillColor(sf::Color::Transparent);
            hbx.setOutlineColor(flcolor::white);
            hbx.setOutlineThickness(-1);
            hbx.setSize({(float)svc::playerLocator.get().collider.bounding_box.shape_w, (float)svc::playerLocator.get().collider.bounding_box.shape_h});
            win.draw(hbx);
        }
        
        //player
        sf::Vector2<float> player_pos = svc::playerLocator.get().apparent_position - svc::cameraLocator.get().physics.position;
        svc::playerLocator.get().render(win, svc::cameraLocator.get().physics.position);
        
        arms::Weapon& curr_weapon = svc::playerLocator.get().loadout.get_equipped_weapon();
        std::vector<sf::Sprite>& curr_weapon_sprites = lookup::weapon_sprites.at(curr_weapon.type);
        sf::Sprite weap_sprite;
        if(!curr_weapon_sprites.empty()) {
            weap_sprite = curr_weapon_sprites.at(arms::WeaponDirLookup.at(curr_weapon.sprite_orientation));
            weap_sprite.setOrigin(NANI_SPRITE_WIDTH/2, NANI_SPRITE_WIDTH/2);
        }
        
        sf::Vector2<float> anchor = svc::playerLocator.get().hand_position;
        sf::Vector2<int> offset = svc::playerLocator.get().loadout.get_equipped_weapon().sprite_offset;
        weap_sprite.setPosition(player_pos.x + anchor.x + offset.x, player_pos.y + anchor.y + offset.y);
        if(map.style == lookup::STYLE::NIGHT) {
            weap_sprite.setColor(flcolor::night);
        }
        win.draw(weap_sprite);
        
        map.render(win, tileset_sprites, svc::cameraLocator.get().physics.position);
        hud.render(win);
        
//        sf::RectangleShape hbx{};
//        hbx.setPosition(20, screen_dimensions.y - 276);
//        hbx.setFillColor(flcolor::goldenrod);
//        hbx.setOutlineColor(flcolor::white);
//        hbx.setOutlineThickness(-1);
//        hbx.setSize({128, 256});
//        win.draw(hbx);
        svc::assetLocator.get().sp_ui_test.setPosition(20, screen_dimensions.y - 148);
        svc::assetLocator.get().sp_bryn_test.setPosition(20, screen_dimensions.y - 276);
//        win.draw(svc::assetLocator.get().sp_ui_test);
//        win.draw(svc::assetLocator.get().sp_bryn_test);
        
        map.transition.render(win);
        
    }
    
    world::Map map{};
    sf::Texture tileset{};
    std::vector<sf::Sprite> tileset_sprites{};
    bool show_colliders = false;
    int x{0};
    
    gui::HUD hud{{20, 20}};
    
};

}

/* GameState_hpp */
