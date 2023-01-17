//
//  AssetManager.hpp
//
//  Created by Alex Frasca on 12/26/22.
//

#pragma once

#ifndef AssetManager_hpp
#define AssetManager_hpp

#include <SFML/Graphics.hpp>
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

namespace fs = std::filesystem;

const uint8_t TILE_WIDTH = 32;

class AssetManager {
public:
    AssetManager() {};
    
    bool importTextures() {
        test_tex.loadFromFile(resource_path + "/test/nani_idle.png");
        t_nani_idle.loadFromFile(resource_path + "/test/nani_idle.png");
        t_tiles_provisional.loadFromFile(resource_path + "/image/tile/provisional_tiles.png");
        t_tiles_shadow.loadFromFile(resource_path + "/image/tile/shadow_tiles.png");
        //load all the other textures...
    }
    
    bool assignSprites() {
        sprite_test.setTexture(test_tex);
        s_nani_idle.setTexture(t_nani_idle);
//        s_nani_idle.setTextureRect(sf::IntRect({ 0, 0 }, { 48, 48 }));
        for(int i = 0; i < 16; ++i) {
            for(int j = 0; j < 16; ++j) {
                sp_tileset.push_back(sf::Sprite());
                //do all tilesets in this loop
                sp_tileset.back().setTexture(t_tiles_provisional);
                sp_tileset.back().setTextureRect(sf::IntRect({j * TILE_WIDTH, i * TILE_WIDTH}, {TILE_WIDTH, TILE_WIDTH}));
            }
        }
        //assign all the other sprites...
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
    sf::Texture test_tex;
    sf::Sprite sprite_test;
    
    sf::Texture t_nani_idle;
    sf::Sprite s_nani_idle;
    
    //load the tilesets!
    sf::Texture t_tiles_provisional;
    sf::Texture t_tiles_overturned;
    sf::Texture t_tiles_ash;
    sf::Texture t_tiles_shadow;
    sf::Texture t_tiles_hoarder;
    sf::Texture t_tiles_abandoned;
    
    std::vector<sf::Sprite> sp_tileset;
    
    std::string resource_path = "";
    
    
};




#endif /* AssetManager_hpp */
