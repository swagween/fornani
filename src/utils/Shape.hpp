//
//  Shape.hpp
//  for_loop
//
//  Created by Alex Frasca on 9/25/19.
//  Copyright © 2019 Western Forest Studios. All rights reserved.
//
#pragma once
#ifndef Shape_hpp
#define Shape_hpp

#include <SFML/Graphics.hpp>
#include <stdio.h>
#include <array>
#include <vector>
#include <cmath>
#include <math.h>
#include <algorithm>

const double error = 0.0001;

class Shape {
    
public:
    
    using Vec = sf::Vector2<float>;
    
    Shape();
    ~Shape() {};
    Shape(std::vector<Vec> verts, std::vector<Vec> edg);
    
    void init();
    void update(double _x, double _y, double _w, double _h);
    void move(double x_, double y_);
    void draw(int camx, int camy);
    Vec get_center();
    
    void handle_collision();
    
    Vec perp(Vec edg);
    
    float getLength(const Vec v);
    Vec getNormalized(const Vec v);
    float dotProduct(const Vec a, const Vec b);
    Vec getNormal(const Vec v);
    Vec projectOnAxis(const std::vector<Vec> vertices, const Vec axis);
    bool areOverlapping(const Vec& a, const Vec& b);
    float getOverlapLength(const Vec& a, const Vec& b);
    Vec getCenter(const Shape& shape);
    Vec getThisCenter();
    std::vector<Vec> getVertices(const Shape& shape);
    Vec getPerpendicularAxis(const std::vector<Vec> vertices, std::size_t index);
    std::array<Vec, 8> getPerpendicularAxes(const std::vector<Vec> vertices1, const std::vector<Vec>& vertices2);
    bool testCollision(const Shape& obb1, const Shape& obb2, Vec& mtv);
    Vec testCollisionGetMTV(const Shape& obb1, const Shape& obb2);
    bool SAT(const Shape& other);
    
    bool AABB_handle_left_collision_static(const Shape& immovable);
    bool AABB_handle_right_collision_static(const Shape& immovable);
    bool AABB_is_left_collision(const Shape& immovable);
    bool AABB_is_right_collision(const Shape& immovable);
    
    std::vector<Vec> vertices;
    std::vector<Vec> edges;
    std::vector<Vec> normals;
    Vec axis;
    
    int shape_x;
    int shape_y;
    int shape_w;
    int shape_h;
    
    float left_offset{};
    float right_offset{};
    
    int num_sides;
    
};

#endif /* Shape_hpp */
