//
//  Shape.hpp
//  for_loop
//
//  Created by Alex Frasca on 9/25/19.
//  Copyright © 2019 Western Forest Studios. All rights reserved.
//
#pragma once

#include <SFML/Graphics.hpp>
#include <stdio.h>
#include <array>
#include <vector>
#include <cmath>
#include <math.h>
#include <algorithm>
#include <cassert>

namespace shape {

    const double error = 0.0001f;

    class Shape {

    public:

        using Vec = sf::Vector2<float>;

        Shape();
        ~Shape() {};
        Shape(std::vector<Vec> verts, std::vector<Vec> edg);
        Shape(Vec dim);

        void init();
        void set_position(const Vec new_pos);
        void update();

        Vec get_center();

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

        //for hurtboxes
        Vec dimensions{};
        Vec position{};
        Vec sprite_offset{};
        int tile_id{};

        int num_sides;

    };

}

/* Shape_hpp */
