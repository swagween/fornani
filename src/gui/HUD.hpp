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

namespace gui {

class HUD {
    
public:
    
    HUD() = default;
    
    void change_health(int amount) {
        filled_hp_cells += amount;
        if(filled_hp_cells > total_hp_cells) {
            filled_hp_cells = total_hp_cells;
        }
        if(filled_hp_cells < 0) {
            filled_hp_cells = 0;
        }
    }
    
    void change_orbs(int amount) {
        num_orbs += amount;
        if(num_orbs > max_orbs) {
            num_orbs = max_orbs;
        }
        if(num_orbs < 0) {
            num_orbs = 0;
        }
    }
    
    void set_gun_display_name(std::string& name) {
        gun_name = name;
    }
    
    int total_hp_cells{};
    int filled_hp_cells{};
    int max_orbs{};
    int num_orbs{};
    std::string gun_name{};
    
}; // end HUD

} // end gui

 /* HUD_hpp */
