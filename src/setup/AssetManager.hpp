//
//  AssetManager.hpp
//
//  Created by Alex Frasca on 12/26/22.
//

#pragma once

#include <filesystem>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <string>
#include <array>

#include "../graphics/FLColor.hpp"
#include "../setup/EnumLookups.hpp"

namespace fs = std::filesystem;

const uint8_t TILE_WIDTH = 32;
const int NANI_SPRITESHEET_WIDTH = 22;
const int NANI_SPRITESHEET_HEIGHT = 10;
const uint8_t NANI_SPRITE_WIDTH = 48;
inline const char* styles[lookup::NUM_STYLES];

class AssetManager {
public:
    AssetManager() {};
    
    void importTextures() {
        t_nani.loadFromFile(resource_path + "/image/character/nani.png");
        t_nani_dark.loadFromFile(resource_path + "/image/character/nani_dark.png");
        t_nani_red.loadFromFile(resource_path + "/image/character/nani_red.png");
        
        t_bryn_test.loadFromFile(resource_path + "/image/portrait/bryn_test.png");
        t_ui_test.loadFromFile(resource_path + "/image/gui/ui_test.png");
        
        t_frdog.loadFromFile(resource_path + "/image/critter/frdog.png");
        
        t_hud.loadFromFile(resource_path + "/image/gui/hud.png");
        t_hud2x.loadFromFile(resource_path + "/image/gui/hud2x.png");
        
        //guns and bullets!
        t_bryns_gun.loadFromFile(resource_path + "/image/weapon/bryns_gun.png");
        t_plasmer.loadFromFile(resource_path + "/image/weapon/plasmer.png");
        t_clover.loadFromFile(resource_path + "/image/weapon/clover.png");
        t_bryns_gun_projectile.loadFromFile(resource_path + "/image/weapon/bryns_gun_proj.png");
        t_plasmer_projectile.loadFromFile(resource_path + "/image/weapon/plasmer_proj.png");
        t_clover_projectile.loadFromFile(resource_path + "/image/weapon/clover_proj.png");
        
        t_hud_elements.loadFromFile(resource_path + "/image/gui/hud_elements.png");
        t_alphabet.loadFromFile(resource_path + "/image/gui/alphabet.png");
        
        t_bg_dusk.loadFromFile(resource_path + "/image/background/dusk.png");
        t_bg_opensky.loadFromFile(resource_path + "/image/background/opensky.png");
        t_bg_overcast.loadFromFile(resource_path + "/image/background/overcast.png");
        t_bg_night.loadFromFile(resource_path + "/image/background/night.png");
        t_bg_dawn.loadFromFile(resource_path + "/image/background/dawn.png");
        t_bg_sunrise.loadFromFile(resource_path + "/image/background/sunrise.png");
        t_bg_rosyhaze.loadFromFile(resource_path + "/image/background/rosyhaze.png");
        t_bg_slime.loadFromFile(resource_path + "/image/background/slime.png");
        t_bg_dirt.loadFromFile(resource_path + "/image/background/dirt.png");
        
        //load all the other textures...
        
        //load tilesets programatically (filenames had better be right...)
        for(int i = 0; i < lookup::NUM_STYLES; ++i) {
            const char* next = lookup::get_style_string.at(lookup::get_style.at(i));
            styles[i] = next;
        }
        for(int i = 0; i < lookup::NUM_STYLES; ++i) {
            tilesets.push_back(sf::Texture());
            std::string style = lookup::get_style_string.at(lookup::get_style.at(i));
            tilesets.back().loadFromFile(resource_path + "/image/tile/" + style + "_tiles.png");
        }
    }
    
    void assignSprites() {
        sp_hud.setTexture(t_hud);
        sp_hud2x.setTexture(t_hud2x);
        
        sp_bryn_test.setTexture(t_bryn_test);
        sp_ui_test.setTexture(t_ui_test);
        
        //gui
        for(int i = 0; i < 4; ++i) {
            sp_hud_elements.push_back(sf::Sprite());
            sp_hud_elements.back().setTexture(t_hud_elements);
        }
        sp_hud_elements.at(0).setTextureRect(sf::IntRect({0, 0}, {2, 18}));
        sp_hud_elements.at(1).setTextureRect(sf::IntRect({2, 0}, {8, 18}));
        sp_hud_elements.at(2).setTextureRect(sf::IntRect({10, 0}, {8, 18}));
        sp_hud_elements.at(3).setTextureRect(sf::IntRect({18, 0}, {14, 18}));
        
        for(int j = 0; j < 2; ++j) {
            for(int i = 0; i < 26; ++i) {
                sp_alphabet.push_back(sf::Sprite(t_alphabet, sf::IntRect({i * 12, j * 12}, {12, 12})));
            }
        }
        
        //gun sprite order convention:          right : left : up_right : down_right : down_left : up_left
        //projectile sprite order convention:   right : left : up : down
        
        //guns and bullets!! (gotta do these all by hand)
        int bg_w = 18; int bg_h = 8; int bg_total_w = 36;
        sp_bryns_gun.push_back(sf::Sprite(t_bryns_gun, sf::IntRect({0,  0},                         {bg_w,  bg_h})));
        sp_bryns_gun.push_back(sf::Sprite(t_bryns_gun, sf::IntRect({bg_total_w - bg_w,  0},         {bg_w,  bg_h})));
        sp_bryns_gun.push_back(sf::Sprite(t_bryns_gun, sf::IntRect({0,  bg_h},                      {bg_h,  bg_w})));
        sp_bryns_gun.push_back(sf::Sprite(t_bryns_gun, sf::IntRect({bg_h, bg_h},                    {bg_h,  bg_w})));
        sp_bryns_gun.push_back(sf::Sprite(t_bryns_gun, sf::IntRect({bg_total_w - bg_h * 2, bg_h},   {bg_h,  bg_w})));
        sp_bryns_gun.push_back(sf::Sprite(t_bryns_gun, sf::IntRect({bg_total_w - bg_h, bg_h},       {bg_h,  bg_w})));
        
        int plas_w = 22; int plas_h = 10; int plas_total_w = 44;
        sp_plasmer.push_back(sf::Sprite(t_plasmer, sf::IntRect({0,  0},                               {plas_w,  plas_h})));
        sp_plasmer.push_back(sf::Sprite(t_plasmer, sf::IntRect({plas_total_w - plas_w,  0},           {plas_w,  plas_h})));
        sp_plasmer.push_back(sf::Sprite(t_plasmer, sf::IntRect({0,  plas_h},                          {plas_h,  plas_w})));
        sp_plasmer.push_back(sf::Sprite(t_plasmer, sf::IntRect({plas_h, plas_h},                      {plas_h,  plas_w})));
        sp_plasmer.push_back(sf::Sprite(t_plasmer, sf::IntRect({plas_total_w - plas_h * 2, plas_h},   {plas_h,  plas_w})));
        sp_plasmer.push_back(sf::Sprite(t_plasmer, sf::IntRect({plas_total_w - plas_h, plas_h},       {plas_h,  plas_w})));
        
        //clover
        int clov_w = 18; int clov_h = 10; int clov_total_w = 40;
        sp_clover.push_back(sf::Sprite(t_clover, sf::IntRect({0,  0},                               {clov_w,  clov_h})));
        sp_clover.push_back(sf::Sprite(t_clover, sf::IntRect({clov_total_w - clov_w,  0},           {clov_w,  clov_h})));
        sp_clover.push_back(sf::Sprite(t_clover, sf::IntRect({0,  clov_h},                          {clov_h,  clov_w})));
        sp_clover.push_back(sf::Sprite(t_clover, sf::IntRect({clov_h, clov_h},                      {clov_h,  clov_w})));
        sp_clover.push_back(sf::Sprite(t_clover, sf::IntRect({clov_total_w - clov_h * 2, clov_h},   {clov_h,  clov_w})));
        sp_clover.push_back(sf::Sprite(t_clover, sf::IntRect({clov_total_w - clov_h, clov_h},       {clov_h,  clov_w})));
        
        int bg_proj_w = 22; int bg_proj_h = 12;
        sp_bryns_gun_projectile.push_back(sf::Sprite(t_bryns_gun_projectile, sf::IntRect({0, 0},                        {bg_proj_w, bg_proj_h})));
        sp_bryns_gun_projectile.push_back(sf::Sprite(t_bryns_gun_projectile, sf::IntRect({0, bg_proj_h},                {bg_proj_w, bg_proj_h})));
        sp_bryns_gun_projectile.push_back(sf::Sprite(t_bryns_gun_projectile, sf::IntRect({bg_proj_w, 0},                {bg_proj_h, bg_proj_w})));
        sp_bryns_gun_projectile.push_back(sf::Sprite(t_bryns_gun_projectile, sf::IntRect({bg_proj_w + bg_proj_w, 0},    {bg_proj_h, bg_proj_w})));
        
        int plas_proj_w = 20; int plas_proj_h = 10;
        sp_plasmer_projectile.push_back(sf::Sprite(t_plasmer_projectile, sf::IntRect({0, 0},                            {plas_proj_w, plas_proj_h})));
        sp_plasmer_projectile.push_back(sf::Sprite(t_plasmer_projectile, sf::IntRect({0, plas_proj_h},                  {plas_proj_w, plas_proj_h})));
        sp_plasmer_projectile.push_back(sf::Sprite(t_plasmer_projectile, sf::IntRect({plas_proj_w, 0},                  {plas_proj_h, plas_proj_w})));
        sp_plasmer_projectile.push_back(sf::Sprite(t_plasmer_projectile, sf::IntRect({plas_proj_w + plas_proj_w, 0},    {plas_proj_h, plas_proj_w})));
        
        int cproj1_w = 6; int cproj1_h = 6; int cproj2_w = 8; int cproj2_h = 6; int cproj3_w = 8; int cproj3_h = 8;
        int cproj4_w = 14; int cproj4_h = 12; int cproj5_w = 8; int cproj5_h = 10; int cproj_total_height = 18;
        sp_clover_projectile.push_back(sf::Sprite(t_clover_projectile, sf::IntRect({0, 0},                                      {cproj1_w, cproj1_h})));
        sp_clover_projectile.push_back(sf::Sprite(t_clover_projectile, sf::IntRect({cproj1_w, 0},                               {cproj2_w, cproj1_h})));
        sp_clover_projectile.push_back(sf::Sprite(t_clover_projectile, sf::IntRect({cproj1_w + cproj2_w, 0},                    {cproj3_w, cproj3_h})));
        sp_clover_projectile.push_back(sf::Sprite(t_clover_projectile, sf::IntRect({0, cproj1_h},                               {cproj4_w, cproj4_h})));
        sp_clover_projectile.push_back(sf::Sprite(t_clover_projectile, sf::IntRect({cproj4_w, cproj3_h},                        {cproj5_w, cproj5_h})));
        sp_clover_projectile.push_back(sf::Sprite(t_clover_projectile, sf::IntRect({0, cproj_total_height},                     {cproj1_w, cproj1_h})));
        sp_clover_projectile.push_back(sf::Sprite(t_clover_projectile, sf::IntRect({cproj1_w, cproj_total_height},              {cproj2_w, cproj2_h})));
        sp_clover_projectile.push_back(sf::Sprite(t_clover_projectile, sf::IntRect({cproj1_w + cproj2_w, cproj_total_height},   {cproj3_w, cproj3_h})));
        sp_clover_projectile.push_back(sf::Sprite(t_clover_projectile, sf::IntRect({0, cproj1_h + cproj_total_height},          {cproj4_w, cproj4_h})));
        sp_clover_projectile.push_back(sf::Sprite(t_clover_projectile, sf::IntRect({cproj4_w, cproj3_h + cproj_total_height},   {cproj5_w, cproj5_h})));
        sp_clover_projectile.push_back(sf::Sprite(t_clover_projectile, sf::IntRect({0, 0},                                      {cproj1_w, cproj1_h})));
        
        sp_clover_projectile.push_back(sf::Sprite(t_clover_projectile, sf::IntRect({cproj1_w, cproj_total_height*2},            {cproj2_w, cproj1_h})));
        sp_clover_projectile.push_back(sf::Sprite(t_clover_projectile, sf::IntRect({cproj1_w + cproj2_w, cproj_total_height*2}, {cproj3_w, cproj3_h})));
        sp_clover_projectile.push_back(sf::Sprite(t_clover_projectile, sf::IntRect({0, cproj1_h + cproj_total_height*2},        {cproj4_w, cproj4_h})));
        sp_clover_projectile.push_back(sf::Sprite(t_clover_projectile, sf::IntRect({cproj4_w, cproj3_h + cproj_total_height*2}, {cproj5_w, cproj5_h})));
        sp_clover_projectile.push_back(sf::Sprite(t_clover_projectile, sf::IntRect({0, cproj_total_height*3},                   {cproj1_w, cproj1_h})));
        sp_clover_projectile.push_back(sf::Sprite(t_clover_projectile, sf::IntRect({cproj1_w, cproj_total_height*3},            {cproj2_w, cproj2_h})));
        sp_clover_projectile.push_back(sf::Sprite(t_clover_projectile, sf::IntRect({cproj1_w + cproj2_w, cproj_total_height*3}, {cproj3_w, cproj3_h})));
        sp_clover_projectile.push_back(sf::Sprite(t_clover_projectile, sf::IntRect({0, cproj1_h + cproj_total_height*3},        {cproj4_w, cproj4_h})));
        sp_clover_projectile.push_back(sf::Sprite(t_clover_projectile, sf::IntRect({cproj4_w, cproj3_h + cproj_total_height*3}, {cproj5_w, cproj5_h})));
        
        //assign all the other sprites...
    }
    
    void load_audio() {
        click_buffer.loadFromFile(resource_path + "/audio/sfx/heavy_click.wav");
        click.setBuffer(click_buffer);
        arms_switch_buffer.loadFromFile(resource_path + "/audio/sfx/arms_switch.wav");
        arms_switch.setBuffer(arms_switch_buffer);
        bg_shot_buffer.loadFromFile(resource_path + "/audio/sfx/bg_shot.wav");
        bg_shot.setBuffer(bg_shot_buffer);
        plasmer_shot_buffer.loadFromFile(resource_path + "/audio/sfx/plasmer_shot.wav");
        plasmer_shot.setBuffer(plasmer_shot_buffer);
        pop_mid_buffer.loadFromFile(resource_path + "/audio/sfx/clover.wav");
        pop_mid.setBuffer(pop_mid_buffer);
        jump_buffer.loadFromFile(resource_path + "/audio/sfx/jump.wav");
        jump.setBuffer(jump_buffer);
        shatter_buffer.loadFromFile(resource_path + "/audio/sfx/shatter.wav");
        shatter.setBuffer(shatter_buffer);
        step_buffer.loadFromFile(resource_path + "/audio/sfx/steps.wav");
        step.setBuffer(step_buffer);
        landed_buffer.loadFromFile(resource_path + "/audio/sfx/landed.wav");
        landed.setBuffer(landed_buffer);
        
        brown_noise.openFromFile(resource_path + "/audio/songs/brown_noise.wav");
        clay_statue.openFromFile(resource_path + "/audio/songs/clay_statue.wav");
        abandoned.openFromFile(resource_path + "/audio/songs/abandoned.wav");
        three_pipes.openFromFile(resource_path + "/audio/songs/three_pipes.wav");
        dusken_cove.openFromFile(resource_path + "/audio/songs/dusken_cove.wav");
    }
    
    void setResourcePath(char** argv) {
        resource_path = find_resources(argv[0]).string();
    }
    
    fs::path find_resources(fs::path exe) {
        auto check = [](fs::path const& prefix) {
            auto path = prefix / "resources";
            if (fs::is_directory(path)) { return path; }
            return fs::path{};
        };
        while (!exe.empty()) {
            if (auto ret = check(exe); !ret.empty()) { return ret; }
            auto parent = exe.parent_path();
            if (exe == parent) { break; }
            exe = std::move(parent);
        }
        return {};
    }
    
    //declare all the textures and sprites as members of the AssetManager
    
    //player and NPCs!
    sf::Texture t_nani{};
    sf::Texture t_nani_dark{};
    sf::Texture t_nani_red{};
    
    //portraits!
    sf::Texture t_bryn_test{};
    sf::Sprite sp_bryn_test{};
    sf::Texture t_ui_test{};
    sf::Sprite sp_ui_test{};
    
    //critters and bosses!
    sf::Texture t_frdog{};
    
    //gui
    sf::Texture t_hud{};
    sf::Texture t_hud2x{};
    sf::Sprite sp_hud{};
    sf::Sprite sp_hud2x{};
    
    sf::Texture t_hud_elements{};
    std::vector<sf::Sprite> sp_hud_elements{};
    
    sf::Texture t_alphabet{};
    std::vector<sf::Sprite> sp_alphabet{};
    
    //tiles!
    std::vector<sf::Texture> tilesets{};
    
    //load scrollingbackground sheets
    sf::Texture t_bg_opensky{};
    sf::Texture t_bg_overcast{};
    sf::Texture t_bg_dusk{};
    sf::Texture t_bg_night{};
    sf::Texture t_bg_dawn{};
    sf::Texture t_bg_sunrise{};
    sf::Texture t_bg_rosyhaze{};
    sf::Texture t_bg_slime{};
    sf::Texture t_bg_black{};
    sf::Texture t_bg_navy{};
    sf::Texture t_bg_dirt{};
    sf::Texture t_bg_gear{};
    sf::Texture t_bg_library{};
    sf::Texture t_bg_granite{};
    sf::Texture t_bg_ruins{};
    sf::Texture t_bg_crevasse{};
    sf::Texture t_bg_deep{};
    sf::Texture t_bg_grove{};
    
    //load the guns and bullets!
    sf::Texture t_bryns_gun{};
    sf::Texture t_bryns_gun_projectile{};
    sf::Texture t_plasmer{};
    sf::Texture t_plasmer_projectile{};
    sf::Texture t_clover{};
    sf::Texture t_clover_projectile{};
    std::vector<sf::Sprite> sp_bryns_gun{};
    std::vector<sf::Sprite> sp_bryns_gun_projectile{};
    std::vector<sf::Sprite> sp_plasmer{};
    std::vector<sf::Sprite> sp_plasmer_projectile{};
    std::vector<sf::Sprite> sp_clover{};
    std::vector<sf::Sprite> sp_clover_projectile{};
    
    //condense these into a 2d vector later
    std::vector<sf::Sprite> sp_tileset_provisional{};
    
    std::string resource_path = "";
    
    
    //sound effects!
    sf::SoundBuffer click_buffer{};
    sf::Sound click;
    
    sf::SoundBuffer arms_switch_buffer{};
    sf::Sound arms_switch;
    sf::SoundBuffer bg_shot_buffer{};
    sf::Sound bg_shot;
    sf::SoundBuffer plasmer_shot_buffer{};
    sf::Sound plasmer_shot;
    sf::SoundBuffer pop_mid_buffer{};
    sf::Sound pop_mid;
    
    sf::SoundBuffer jump_buffer{};
    sf::Sound jump;
    sf::SoundBuffer shatter_buffer{};
    sf::Sound shatter;
    sf::SoundBuffer step_buffer{};
    sf::Sound step;
    sf::SoundBuffer landed_buffer{};
    sf::Sound landed;
    
    //songs!
    sf::Music brown_noise{};
    sf::Music clay_statue{};
    sf::Music abandoned{};
    sf::Music three_pipes{};
    sf::Music dusken_cove{};
    
    //other members
    int music_vol{24};
    
    
};

/* AssetManager_hpp */
