#ifndef RF_UTIL_DIJKSTRA_HPP
#define RF_UTIL_DIJKSTRA_HPP

#include <cstddef>
#include <climits>

#include <rf/util/Vec2.hpp>
#include <rf/util/Map.hpp>

namespace rf {
  struct DijkstraMap {
    public:
    static constexpr unsigned int infinity = UINT_MAX;

    DijkstraMap() = default;
    DijkstraMap(const DijkstraMap & other) = delete;
    DijkstraMap & operator=(const DijkstraMap & other) = delete;
    ~DijkstraMap();

    void compute(const Map<unsigned int> & costs, Vec2u start);
    const Map<unsigned int> & distances() { return _distances; }

    static void test_heap();
    static void test();

    private:
    struct GraphNode;
    typedef GraphNode * HeapNode;
    struct GraphNode {
      // connectivity with neighboring cells
      GraphNode * neighbors[8] = {
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr
      };
      // determines heap order; should not be set directly
      unsigned int distance = 0;
      // cost to enter this cell
      unsigned int cost = 0;
      // pointer to location in heap
      HeapNode * heap_node = nullptr;
    };

    GraphNode * graph_nodes = nullptr;
    HeapNode * heap_nodes = nullptr;
    size_t graph_size = 0;
    size_t heap_size = 0;

    Map<unsigned int> _distances;

    void init(const Map<unsigned int> & costs, Vec2u start);

    GraphNode * heap_pop();
    void heap_decrease(GraphNode * node, unsigned int distance);
  };
}

#endif
