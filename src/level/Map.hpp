//
//  Map.hpp
//  for_loop
//
//  Created by Alex Frasca on 10/6/18.
//  Copyright © 2018 Western Forest Studios. All rights reserved.
//
#pragma once

#include "../utils/Grid.hpp"
#include "../utils/Shape.hpp"
#include "../utils/Random.hpp"
#include <string>
#include <vector>
#include <fstream>
#include "../weapon/Projectile.hpp"
#include "../setup/LookupTables.hpp"
#include "../setup/MapLookups.hpp"
#include "../graphics/Background.hpp"
#include "../graphics/Transition.hpp"
#include "../entities/critter/Bestiary.hpp"
#include "../entities/world/Portal.hpp"
#include "../entities/world/Inspectable.hpp"

const int NUM_LAYERS{8};
const int CHUNK_SIZE{16};
const int CELL_SIZE{32};

namespace world {

    /*ElementBehavior {rate, rate_variance, expulsion_force, expulsion_variance, cone, grav, grav_variance, x_friction, y_friction }; */
    //map emitters!
    constexpr inline vfx::ElementBehavior breakable_spray{ 2, 1, 1.5, 0.8, 0.8, 0.01, 0.005, 0.99, 0.99 };
    constexpr inline vfx::EmitterStats breakable_stats{ 10, 0, 80, 60, 3.0f };
    inline auto breakable_debris = vfx::Emitter(breakable_spray, breakable_stats, flcolor::goldenrod);
    constexpr inline vfx::ElementBehavior player_death_spray{ 10, 2, 1.8, 1.7, 0.8, 0.008, 0.001, 0.99, 0.99 };
    constexpr inline vfx::EmitterStats player_death_stats{ 4, 0, 80, 60, 4.0f };
    inline auto player_death = vfx::Emitter(player_death_spray, player_death_stats, flcolor::white);

enum LAYER_ORDER {
    BACKGROUND = 0,
    MIDDLEGROUND = 4,
    FOREGROUND = 7,
};

// a Layer is a grid with a render priority and a flag to determine if scene entities can collide with it.
// for for loop, the current convention is that the only collidable layer is layer 4 (index 3), or the middleground.

class Layer {
    
public:
    
    Layer() = default;
    Layer(uint8_t o, bool c, sf::Vector2<uint16_t> dim) : render_order(o), collidable(c), dimensions(dim) {
        grid = squid::Grid({dim.x, dim.y});
    }
    squid::Grid grid{};
    uint8_t render_order{};
    bool collidable{};
    sf::Vector2<uint16_t> dimensions{};
    
};

// a Map is just a set of layers that will render on top of each other

class Map {
    
public:
    
    using Vec = sf::Vector2<float>;
    using Vecu16 = sf::Vector2<uint16_t>;
    
    Map();
    //methods
    void load(const std::string& path);
    void update();
    void render(sf::RenderWindow& win, std::vector<sf::Sprite>& tileset, sf::Vector2<float> cam);
    void render_background(sf::RenderWindow& win, std::vector<sf::Sprite>& tileset, sf::Vector2<float> cam);
    squid::Tile& tile_at(const uint8_t i, const uint8_t j);
    Shape& shape_at(const uint8_t i, const uint8_t j);
    void spawn_projectile_at(sf::Vector2<float> pos);
    void manage_projectiles();
    Vec get_spawn_position(int portal_source_map_id);
    
    //layers
    std::vector<Layer> layers;
    Vec real_dimensions{}; // pixel dimensions (maybe useless)
    Vecu16 dimensions{}; // points on the 32x32-unit grid
    Vecu16 chunk_dimensions{}; // how many chunks (16x16 squares) in the room
    
    //entities
    std::vector<arms::Projectile> active_projectiles{};
    std::vector<vfx::Emitter> active_emitters{};
    std::vector<critter::Critter> critters{};
    std::vector<entity::Portal> portals{};
    std::vector<entity::Inspectable> inspectables{};
    
    std::unique_ptr<bg::Background> background{};
    flfx::Transition transition{255};
    
    critter::Bestiary bestiary{};
    
    lookup::STYLE style{}; // which tileset to render

    //minimap
    sf::View minimap{};
    sf::RectangleShape minimap_tile{};
    sf::RectangleShape borderbox{};
    int bg{}; // which background to render
    
    int room_id{}; // should be assigned to its constituent chunks
    bool game_over{ false };
    bool show_minimap{ false };
    
};

}
/* Map_hpp */
