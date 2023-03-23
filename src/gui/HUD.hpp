//
//  HUD.hpp
//  fornani
//
//  Created by Alex Frasca on 12/26/22.
//

#pragma once

#include <cstdio>
#include <memory>
#include <string>
#include <list>
#include "../setup/ServiceLocator.hpp"
#include "../setup/EnumLookups.hpp"

namespace gui {

enum ELEMS {
    PIPE = 0,
    HP = 1,
    HP_EMPTY = 2,
    TEXT = 3
};

inline const sf::Vector2<int> HP_origin{36, 0};
inline const sf::Vector2<int> ORB_origin{48, 24};
inline const sf::Vector2<int> GUN_origin{48, 48};
inline const int PAD{6};

class HUD {
    
public:
    
    HUD(sf::Vector2<int> pos) : position(pos) { update(); }
    
    void update() {
        filled_hp_cells = svc::playerLocator.get().player_stats.health;
        num_orbs = svc::playerLocator.get().player_stats.orbs;
        total_hp_cells = svc::playerLocator.get().player_stats.max_health;
        max_orbs = svc::playerLocator.get().player_stats.max_orbs;
        gun_name = svc::playerLocator.get().loadout.get_equipped_weapon().label;
        constrain();
    }
    
    void constrain() {
        if(filled_hp_cells > total_hp_cells) {
            filled_hp_cells = total_hp_cells;
        }
        if(filled_hp_cells < 0) {
            filled_hp_cells = 0;
        }
        if(num_orbs > max_orbs) {
            num_orbs = max_orbs;
        }
        if(num_orbs < 0) {
            num_orbs = 0;
        }
    }
    
    void render(sf::RenderWindow& win) {
        svc::assetLocator.get().sp_hud.setPosition(position.x, position.y);
        win.draw(svc::assetLocator.get().sp_hud);
        
        //beginning pipes
        svc::assetLocator.get().sp_hud_elements.at(PIPE).setPosition(position.x + HP_origin.x + PAD, position.y);
        win.draw(svc::assetLocator.get().sp_hud_elements.at(PIPE));
        svc::assetLocator.get().sp_hud_elements.at(PIPE).setPosition(position.x + ORB_origin.x + PAD, position.y + ORB_origin.y);
        win.draw(svc::assetLocator.get().sp_hud_elements.at(PIPE));
        svc::assetLocator.get().sp_hud_elements.at(PIPE).setPosition(position.x + GUN_origin.x + PAD, position.y + GUN_origin.y);
        win.draw(svc::assetLocator.get().sp_hud_elements.at(PIPE));
        
        //HP
        for(int i = 0; i < total_hp_cells; ++i) {
            if(i < filled_hp_cells) {
                svc::assetLocator.get().sp_hud_elements.at(HP).setPosition(position.x + HP_origin.x + PAD + (8 * i) + 2, position.y);
                win.draw(svc::assetLocator.get().sp_hud_elements.at(HP));
            } else {
                svc::assetLocator.get().sp_hud_elements.at(HP_EMPTY).setPosition(position.x + HP_origin.x + PAD + (8 * i) + 2, position.y);
                win.draw(svc::assetLocator.get().sp_hud_elements.at(HP_EMPTY));
            }
        }
        
        //ORB
        digits = std::to_string(num_orbs);
        int ctr{0};
        for(auto& digit : digits) {
            svc::assetLocator.get().sp_hud_elements.at(TEXT).setPosition(position.x + ORB_origin.x + PAD + (12 * ctr) + 2, position.y + ORB_origin.y);
            win.draw(svc::assetLocator.get().sp_hud_elements.at(TEXT));
            
            svc::assetLocator.get().sp_alphabet.at(lookup::get_character.at(('0' + digit))).setPosition(position.x + ORB_origin.x + PAD + (12 * ctr) + 2, position.y + ORB_origin.y + 2);
            win.draw(svc::assetLocator.get().sp_alphabet.at(lookup::get_character.at(('0' + digit))));
            
            ctr++;
        }
        //end pipe
        svc::assetLocator.get().sp_hud_elements.at(PIPE).setPosition(position.x + ORB_origin.x + PAD + (12 * ctr) + 4, position.y + ORB_origin.y);
        win.draw(svc::assetLocator.get().sp_hud_elements.at(PIPE));
        
        //GUN
        ctr = 0;
        for(auto& letter : gun_name) {
            svc::assetLocator.get().sp_hud_elements.at(TEXT).setPosition(position.x + GUN_origin.x + PAD + (12 * ctr) + 2, position.y + GUN_origin.y);
            win.draw(svc::assetLocator.get().sp_hud_elements.at(TEXT));
            if(!std::isspace(letter)) {
                svc::assetLocator.get().sp_alphabet.at(lookup::get_character.at(letter)).setPosition(position.x + GUN_origin.x + PAD + (12 * ctr) + 2, position.y + GUN_origin.y + 2);
                win.draw(svc::assetLocator.get().sp_alphabet.at(lookup::get_character.at(letter)));
            }
            ctr++;
        }
        //end pipe
        svc::assetLocator.get().sp_hud_elements.at(PIPE).setPosition(position.x + GUN_origin.x + PAD + (12 * ctr) + 4, position.y + GUN_origin.y);
        win.draw(svc::assetLocator.get().sp_hud_elements.at(PIPE));
    }
    
    sf::Vector2<int> position{};
    int total_hp_cells{};
    int filled_hp_cells{};
    int max_orbs{};
    int num_orbs{};
    std::string_view gun_name{};
    std::string_view digits{};
    
}; // end HUD

} // end gui

 /* HUD_hpp */
