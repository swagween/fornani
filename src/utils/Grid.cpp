//
//  Grid.cpp
//  squid
//
//  Created by Alex Frasca on 12/26/22.
//

#include "Grid.hpp"

squid::Grid::Grid() {
    spacing = DEFAULT_SPACING;
}

squid::Grid::Grid(sf::Vector2<uint32_t> d) : dimensions(d) {
    
    spacing = DEFAULT_SPACING;
    
    initialize();
    
}

void squid::Grid::initialize() {
    
    cells.clear();
    
    for(int i = 0; i < dimensions.x * dimensions.y; i++) {
        
        // calculate index values
        uint32_t xidx = std::floor(i%dimensions.x);
        uint32_t yidx = std::floor(i/dimensions.x);
        
        //calculate positions with offsets
        float xpos, ypos;
        if(yidx % 2 == 0) {
            xpos = xidx*spacing;
        } else {
            xpos = xidx*spacing;
        }
        
        if(xidx % 2 == 0) {
            ypos = yidx*spacing;
        } else {
            ypos = yidx*spacing;
        }
        cells.push_back(Tile({xidx, yidx}, {xpos, ypos}, 0, lookup::TILE_TYPE::TILE_BASIC));
        cells.back().position = sf::Vector2<float>(xpos, ypos);
        cells.back().bounding_box.set_position(sf::Vector2<float>(xpos, ypos));
        
    }
    
    init_shape_vertices();
    
}

void squid::Grid::update() {
    
    for(int i = 0; i < dimensions.x * dimensions.y; i++) {
        
        push_cells(i);
        
    }
    
}

void squid::Grid::set_spacing(float spc) {
    
    spacing = spc;
    for(int i = 0; i < dimensions.x * dimensions.y; i++) {
        float xpos = cells.at(i).index.x * spacing;
        float ypos = cells.at(i).index.y * spacing;
        cells.at(i).position = sf::Vector2<float>(xpos, ypos);
    }
    
}

void squid::Grid::push_cells(int i) {
    // calculate index values
    uint32_t xidx = std::floor(i % dimensions.x);
    uint32_t yidx = std::floor(i / dimensions.x);

    //calculate positions with offsets
    float xpos, ypos;
    if (yidx % 2 == 0) {
        xpos = xidx * spacing;
    }
    else {
        xpos = xidx * spacing;
    }

    if (xidx % 2 == 0) {
        ypos = yidx * spacing;
    }
    else {
        ypos = yidx * spacing;
    }

    //populate the grid
    cells.at(i).index = sf::Vector2<uint32_t>(xidx, yidx);
    cells.at(i).position = sf::Vector2<float>(xpos, ypos);

    cells.at(i).bounding_box.set_position(sf::Vector2<float>(xpos, ypos));
}

void squid::Grid::init_shape_vertices() {
    float _WIDTH = 32.f;
    for (auto& tile : cells) {
        //check vector bounds
        if(tile.bounding_box.vertices.size() >= 4) {
            //this function creates slants for appropriate tiles
            switch (tile.value) {
                    //top left long ramp
                case CEIL_SLANT_INDEX:
                    tile.bounding_box.vertices[2].y -= _WIDTH/4;
                    break;
                case CEIL_SLANT_INDEX + 1:
                    tile.bounding_box.vertices[2].y -= _WIDTH/2;
                    tile.bounding_box.vertices[3].y -= _WIDTH/4;
                    break;
                case CEIL_SLANT_INDEX + 2:
                    tile.bounding_box.vertices[2].y -= _WIDTH - _WIDTH/4;
                    tile.bounding_box.vertices[3].y -= _WIDTH/2;
                    break;
                case CEIL_SLANT_INDEX + 3:
                    tile.bounding_box.vertices[2].y -= _WIDTH - shape::error;
                    tile.bounding_box.vertices[3].y -= _WIDTH - _WIDTH/4;
                    break;
                    //top right long ramp
                case CEIL_SLANT_INDEX + 7:
                    tile.bounding_box.vertices[3].y -= _WIDTH/4;
                    break;
                case CEIL_SLANT_INDEX + 6:
                    tile.bounding_box.vertices[3].y -= _WIDTH/2;
                    tile.bounding_box.vertices[2].y -= _WIDTH/4;
                    break;
                case CEIL_SLANT_INDEX + 5:
                    tile.bounding_box.vertices[3].y -= _WIDTH - _WIDTH/4;
                    tile.bounding_box.vertices[2].y -= _WIDTH/2;
                    break;
                case CEIL_SLANT_INDEX + 4:
                    tile.bounding_box.vertices[3].y -= _WIDTH - shape::error;
                    tile.bounding_box.vertices[2].y -= _WIDTH - _WIDTH/4;
                    break;
                    //top left short ramp 1
                case CEIL_SLANT_INDEX + 8:
                    tile.bounding_box.vertices[2].y -= _WIDTH/2;
                    break;
                case CEIL_SLANT_INDEX + 9:
                    tile.bounding_box.vertices[3].y -= _WIDTH/2;
                    tile.bounding_box.vertices[2].y -= _WIDTH - shape::error;
                    break;
                    //top right short ramp 1
                case CEIL_SLANT_INDEX + 11:
                    tile.bounding_box.vertices[3].y -= _WIDTH/2;
                    break;
                case CEIL_SLANT_INDEX + 10:
                    tile.bounding_box.vertices[2].y -= _WIDTH/2;
                    tile.bounding_box.vertices[3].y -= _WIDTH - shape::error;
                    break;
                    //top left short ramp 2
                case CEIL_SLANT_INDEX + 12:
                    tile.bounding_box.vertices[2].y -= _WIDTH/2;
                    break;
                case CEIL_SLANT_INDEX + 13:
                    tile.bounding_box.vertices[3].y -= _WIDTH/2;
                    tile.bounding_box.vertices[2].y -= _WIDTH - shape::error;
                    break;
                    //top right short ramp 2
                case CEIL_SLANT_INDEX + 15:
                    tile.bounding_box.vertices[3].y -= _WIDTH/2;
                    break;
                case CEIL_SLANT_INDEX + 14:
                    tile.bounding_box.vertices[2].y -= _WIDTH/2;
                    tile.bounding_box.vertices[3].y -= _WIDTH - shape::error;
                    break;
                    //bottom left long ramp
                case FLOOR_SLANT_INDEX:
                    tile.bounding_box.vertices[1].y += _WIDTH/4;
                    break;
                case FLOOR_SLANT_INDEX + 1:
                    tile.bounding_box.vertices[0].y += _WIDTH/4;
                    tile.bounding_box.vertices[1].y += _WIDTH/2;
                    break;
                case FLOOR_SLANT_INDEX + 2:
                    tile.bounding_box.vertices[0].y += _WIDTH/2;
                    tile.bounding_box.vertices[1].y += _WIDTH - _WIDTH/4;
                    break;
                case FLOOR_SLANT_INDEX + 3:
                    tile.bounding_box.vertices[0].y += _WIDTH - _WIDTH/4;
                    tile.bounding_box.vertices[1].y += _WIDTH - shape::error;
                    break;
                    //bottom right long ramp
                case FLOOR_SLANT_INDEX + 7:
                    tile.bounding_box.vertices[0].y += _WIDTH/4;
                    break;
                case FLOOR_SLANT_INDEX + 6:
                    tile.bounding_box.vertices[1].y += _WIDTH/4;
                    tile.bounding_box.vertices[0].y += _WIDTH/2;
                    break;
                case FLOOR_SLANT_INDEX + 5:
                    tile.bounding_box.vertices[1].y += _WIDTH/2;
                    tile.bounding_box.vertices[0].y += _WIDTH - _WIDTH/4;
                    break;
                case FLOOR_SLANT_INDEX + 4:
                    tile.bounding_box.vertices[1].y += _WIDTH - _WIDTH/4;
                    tile.bounding_box.vertices[0].y += _WIDTH - shape::error;
                    break;
                    //bottom left short ramp 1
                case FLOOR_SLANT_INDEX + 8:
                    tile.bounding_box.vertices[1].y += _WIDTH/2;
                    break;
                case FLOOR_SLANT_INDEX + 9:
                    tile.bounding_box.vertices[0].y += _WIDTH/2;
                    tile.bounding_box.vertices[1].y += _WIDTH - shape::error;
                    break;
                    //bottom right short ramp 1
                case FLOOR_SLANT_INDEX + 11:
                    tile.bounding_box.vertices[0].y += _WIDTH/2;
                    break;
                case FLOOR_SLANT_INDEX + 10:
                    tile.bounding_box.vertices[1].y += _WIDTH/2;
                    tile.bounding_box.vertices[0].y += _WIDTH - shape::error;
                    break;
                    //bottom left short ramp 2
                case FLOOR_SLANT_INDEX + 12:
                    tile.bounding_box.vertices[1].y += _WIDTH/2;
                    break;
                case FLOOR_SLANT_INDEX + 13:
                    tile.bounding_box.vertices[0].y += _WIDTH/2;
                    tile.bounding_box.vertices[1].y += _WIDTH - shape::error;
                    break;
                    //bottom right short ramp 2
                case FLOOR_SLANT_INDEX + 15:
                    tile.bounding_box.vertices[0].y += _WIDTH/2;
                    break;
                case FLOOR_SLANT_INDEX + 14:
                    tile.bounding_box.vertices[1].y += _WIDTH/2;
                    tile.bounding_box.vertices[0].y += _WIDTH - shape::error;
                    break;
                default:
                    break;
            }
            for(int i = 0; i < tile.bounding_box.vertices.size(); i++) {
                tile.bounding_box.edges[i].x = tile.bounding_box.vertices[(i + 1)%tile.bounding_box.vertices.size()].x - tile.bounding_box.vertices[i].x;
                tile.bounding_box.edges[i].y = tile.bounding_box.vertices[(i + 1)%tile.bounding_box.vertices.size()].y - tile.bounding_box.vertices[i].y;
                tile.bounding_box.normals[i] = tile.bounding_box.perp(tile.bounding_box.edges[i]);
            }
        }
    }
}
