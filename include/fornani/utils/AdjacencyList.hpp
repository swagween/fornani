
#pragma once

#include <cstdio>
#include <list>
#include <memory>

namespace {

template <typename T>
class AdjacencyList {

  public:
	AdjacencyList() = default;
	~AdjacencyList() { adjlist.clear(); }
	AdjacencyList& operator=(AdjacencyList&&) = delete;

	void add_edge(T& u, T& v, bool bidirectional) {
		adjlist.push_back(u);
		if (bidirectional) { adjlist.push_back(v); }
	}

	std::list<T> adjlist{};
	int size{};
};

} // namespace
