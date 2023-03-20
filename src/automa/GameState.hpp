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
    virtual void setTilesetTexture(sf::Texture& t) {};
    virtual void handle_events(sf::Event& event) {
        
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
    void setTilesetTexture(sf::Texture& t) {
    }
    void handle_events(sf::Event& event) {
        
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
            }
        }
//        svc::assetLocator.get().abandoned.setVolume(50);
//        svc::assetLocator.get().abandoned.play();
//        svc::assetLocator.get().abandoned.setLoop(true);
        
//        svc::assetLocator.get().three_pipes.setVolume(svc::assetLocator.get().music_vol);
//        svc::assetLocator.get().three_pipes.play();
//        svc::assetLocator.get().three_pipes.setLoop(true);
        
        svc::assetLocator.get().brown_noise.setVolume(20);
        svc::assetLocator.get().brown_noise.play();
        svc::assetLocator.get().brown_noise.setLoop(true);
    }
    void handle_events(sf::Event& event) {
        svc::playerLocator.get().handle_events(event);
        if (event.type == sf::Event::EventType::KeyPressed) {
            if (event.key.code == sf::Keyboard::H) {
                show_colliders = !show_colliders;
            }
        }
    }
    
    void logic(Time dt) {
        map.update();
        hud.update();
        svc::cameraLocator.get().center(svc::playerLocator.get().anchor_point);
        svc::cameraLocator.get().update();
        svc::cameraLocator.get().restrict_movement(map.real_dimensions);
        svc::playerLocator.get().update(dt);
        svc::assetLocator.get().three_pipes.setVolume(svc::assetLocator.get().music_vol);
    }
    
    void render(sf::RenderWindow& win) {
        
        map.render_background(win, tileset_sprites, svc::cameraLocator.get().physics.position);
        
        if(show_colliders) {
            sf::Vector2<float> jumpbox_pos = sf::operator-(svc::playerLocator.get().jumpbox.vertices.at(0), svc::cameraLocator.get().physics.position);
            sf::RectangleShape jbx{};
            jbx.setPosition(jumpbox_pos.x, jumpbox_pos.y);
            jbx.setFillColor(sf::Color::Transparent);
            jbx.setOutlineColor(sf::Color(235, 232, 249, 80));
            jbx.setOutlineThickness(-1);
            jbx.setSize({(float)svc::playerLocator.get().jumpbox.shape_w, (float)svc::playerLocator.get().jumpbox.shape_h});
            win.draw(jbx);
            
            sf::Vector2<float> leftbox_pos = sf::operator-(svc::playerLocator.get().left_detector.vertices.at(0), svc::cameraLocator.get().physics.position);
            sf::RectangleShape leftbox{};
            leftbox.setPosition(leftbox_pos.x, leftbox_pos.y);
            leftbox.setFillColor(sf::Color::Transparent);
            leftbox.setOutlineColor(sf::Color(235, 232, 249, 80));
            leftbox.setOutlineThickness(-1);
            leftbox.setSize({(float)svc::playerLocator.get().left_detector.shape_w, (float)svc::playerLocator.get().left_detector.shape_h});
            win.draw(leftbox);
            
            sf::Vector2<float> rightbox_pos = sf::operator-(svc::playerLocator.get().right_detector.vertices.at(0), svc::cameraLocator.get().physics.position);
            sf::RectangleShape rightbox{};
            rightbox.setPosition(rightbox_pos.x, rightbox_pos.y);
            rightbox.setFillColor(sf::Color::Transparent);
            rightbox.setOutlineColor(sf::Color(235, 232, 249, 80));
            rightbox.setOutlineThickness(-1);
            rightbox.setSize({(float)svc::playerLocator.get().right_detector.shape_w, (float)svc::playerLocator.get().right_detector.shape_h});
            win.draw(rightbox);
            
            sf::Vector2<float> wallbox_pos = sf::operator-(svc::playerLocator.get().wall_slide_detector.vertices.at(0), svc::cameraLocator.get().physics.position);
            sf::RectangleShape wallbox{};
            wallbox.setPosition(wallbox_pos.x, wallbox_pos.y);
            wallbox.setFillColor(sf::Color::Transparent);
            wallbox.setOutlineColor(sf::Color(23, 232, 249, 80));
            wallbox.setOutlineThickness(-1);
            wallbox.setSize({(float)svc::playerLocator.get().wall_slide_detector.shape_w, (float)svc::playerLocator.get().wall_slide_detector.shape_h});
            win.draw(wallbox);
            
            sf::Vector2<float> predictive_hurtbox_pos = sf::operator-(svc::playerLocator.get().predictive_hurtbox.vertices.at(0), svc::cameraLocator.get().physics.position);
            sf::RectangleShape predictive_hbx{};
            predictive_hbx.setPosition(predictive_hurtbox_pos.x, predictive_hurtbox_pos.y);
            predictive_hbx.setFillColor(sf::Color::Transparent);
            predictive_hbx.setOutlineColor(flcolor::fucshia);
            predictive_hbx.setOutlineThickness(-1);
            predictive_hbx.setSize({(float)svc::playerLocator.get().predictive_hurtbox.shape_w, (float)svc::playerLocator.get().predictive_hurtbox.shape_h});
            win.draw(predictive_hbx);
            
            sf::Vector2<float> hurtbox_pos = sf::operator-(svc::playerLocator.get().hurtbox.vertices.at(0), svc::cameraLocator.get().physics.position);
            sf::RectangleShape hbx{};
            hbx.setPosition(hurtbox_pos.x, hurtbox_pos.y);
            hbx.setFillColor(sf::Color::Transparent);
            hbx.setOutlineColor(flcolor::white);
            hbx.setOutlineThickness(-1);
            hbx.setSize({(float)svc::playerLocator.get().hurtbox.shape_w, (float)svc::playerLocator.get().hurtbox.shape_h});
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
        
        
    }
    
    world::Map map{};
    sf::Texture tileset{};
    std::vector<sf::Sprite> tileset_sprites{};
    bool show_colliders = false;
    
    gui::HUD hud{{20, 20}};
    
};

}

/* GameState_hpp */
