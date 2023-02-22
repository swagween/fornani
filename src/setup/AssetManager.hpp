//
//  AssetManager.hpp
//
//  Created by Alex Frasca on 12/26/22.
//

#pragma once

#ifndef AssetManager_hpp
#define AssetManager_hpp

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <unistd.h>
#include <stdio.h>
#include <string>

#if defined(_WIN32)
#include <windows.h>
#include <Shlwapi.h>
#include <io.h>

#define access _access_s
#endif

#ifdef __APPLE__
#include <libgen.h>
#include <limits.h>
#include <mach-o/dyld.h>
#include <unistd.h>
#endif

#ifdef __linux__
#include <limits.h>
#include <libgen.h>
#include <unistd.h>

#if defined(__sun)
#define PROC_SELF_EXE "/proc/self/path/a.out"
#else
#define PROC_SELF_EXE "/proc/self/exe"
#endif

#endif

#include <filesystem>

#include "../graphics/FLColor.hpp"

namespace fs = std::filesystem;

const uint8_t TILE_WIDTH = 32;
const int NANI_SPRITESHEET_WIDTH = 22;
const int NANI_SPRITESHEET_HEIGHT = 10;
const uint8_t NANI_SPRITE_WIDTH = 48;

class AssetManager {
public:
    AssetManager() {};
    
    bool importTextures() {
        test_tex.loadFromFile(resource_path + "/test/nani_idle.png");
        t_nani_spritesheet.loadFromFile(resource_path + "/image/character/nani_spritesheet.png");
        t_tiles_provisional.loadFromFile(resource_path + "/image/tile/provisional_tiles.png");
        t_tiles_shadow.loadFromFile(resource_path + "/image/tile/shadow_tiles.png");
        t_hud.loadFromFile(resource_path + "/image/gui/hud.png");
        t_hud2x.loadFromFile(resource_path + "/image/gui/hud2x.png");
        
        //guns and bullets!
        t_bryns_gun.loadFromFile(resource_path + "/image/weapon/bryns_gun.png");
        t_plasmer.loadFromFile(resource_path + "/image/weapon/plasmer.png");
        t_clover.loadFromFile(resource_path + "/image/weapon/clover.png");
        t_bryns_gun_projectile.loadFromFile(resource_path + "/image/weapon/bryns_gun_proj.png");
        t_plasmer_projectile.loadFromFile(resource_path + "/image/weapon/plasmer_proj.png");
        t_clover_projectile.loadFromFile(resource_path + "/image/weapon/clover_proj.png");
        //load all the other textures...
    }
    
    bool assignSprites() {
        sprite_test.setTexture(test_tex);
        sp_hud.setTexture(t_hud);
        sp_hud2x.setTexture(t_hud2x);
//        s_nani_idle.setTextureRect(sf::IntRect({ 0, 0 }, { 48, 48 }));
        for(int i = 0; i < 16; ++i) {
            for(int j = 0; j < 16; ++j) {
                sp_tileset_provisional.push_back(sf::Sprite());
                sp_tileset_shadow.push_back(sf::Sprite());
                //do all tilesets in this loop
                sp_tileset_provisional.back().setTexture(t_tiles_provisional);
                sp_tileset_shadow.back().setTexture(t_tiles_shadow);
                sp_tileset_provisional.back().setTextureRect(sf::IntRect({j * TILE_WIDTH, i * TILE_WIDTH}, {TILE_WIDTH, TILE_WIDTH}));
                sp_tileset_shadow.back().setTextureRect(sf::IntRect({j * TILE_WIDTH, i * TILE_WIDTH}, {TILE_WIDTH, TILE_WIDTH}));
            }
        }
        
        for(int i = 0; i < NANI_SPRITESHEET_WIDTH; ++i) {
            for(int j = 0; j < NANI_SPRITESHEET_HEIGHT; ++j) {
                sp_nani.push_back(sf::Sprite());
                sp_nani.back().setTexture(t_nani_spritesheet);
                sp_nani.back().setTextureRect(sf::IntRect({i * NANI_SPRITE_WIDTH, j * NANI_SPRITE_WIDTH}, {NANI_SPRITE_WIDTH, NANI_SPRITE_WIDTH}));
                sp_nani.back().setTextureRect(sf::IntRect({i * NANI_SPRITE_WIDTH, j * NANI_SPRITE_WIDTH}, {NANI_SPRITE_WIDTH, NANI_SPRITE_WIDTH}));
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
    
    bool load_audio() {
//        click_buffer.loadFromFile(resource_path + "/audio/sfx/click.wav");
//        click.setBuffer(click_buffer);
    }
    
    void setResourcePath(char** argv) {
        resource_path = find_resources(argv[0]);
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
    sf::Texture test_tex{};
    sf::Sprite sprite_test{};
    
    sf::Texture t_nani_spritesheet{};
    std::vector<sf::Sprite> sp_nani{};
    
    //gui
    sf::Texture t_hud{};
    sf::Texture t_hud2x{};
    sf::Sprite sp_hud{};
    sf::Sprite sp_hud2x{};
    
    //load the tilesets!
    sf::Texture t_tiles_provisional{};
    sf::Texture t_tiles_overturned{};
    sf::Texture t_tiles_ash{};
    sf::Texture t_tiles_shadow{};
    sf::Texture t_tiles_hoarder{};
    sf::Texture t_tiles_abandoned{};
    
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
    std::vector<sf::Sprite> sp_tileset_shadow{};
    std::vector<sf::Sprite> sp_tileset_abandoned{};
    
    std::string resource_path = "";
    
//    sf::SoundBuffer click_buffer{};
//    sf::Sound click;
    
};




#endif /* AssetManager_hpp */
