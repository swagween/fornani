//
//  AdjacencyList.hpp
//  fornani
//
//  Created by Alex Frasca on 12/26/22.
//

#pragma once

#include <cstdio>
#include <memory>
#include <list>

namespace {

template<typename T>
class AdjacencyList {
    
public:
    
    AdjacencyList() = default;
    ~AdjacencyList() { adjlist.clear(); }
    AdjacencyList& operator=(AdjacencyList&&) = delete;
    
    void add_edge(T& u, T& v, bool bidirectional) {
        adjlist.push_back(u);
        if(bidirectional) { adjlist.push_back(v); }
    }
    
    std::list<T> adjlist{};
    int size{};
    
}; // End AdjacencyList

}

 /* AdjacencyList_hpp */
