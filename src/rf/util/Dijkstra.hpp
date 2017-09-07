#ifndef RF_UTIL_DIJKSTRA_HPP
#define RF_UTIL_DIJKSTRA_HPP

#include <cstddef>
#include <limits>

#include <rf/util/Vec2.hpp>
#include <rf/util/Map.hpp>

namespace rf {
  struct DijkstraMap {
    public:
    typedef int Distance;
    static constexpr Distance infinity = std::numeric_limits<Distance>::max();
    typedef std::pair<Vec2u, int> Goal;

    DijkstraMap() = default;
    DijkstraMap(const DijkstraMap & other) = delete;
    DijkstraMap & operator=(const DijkstraMap & other) = delete;
    ~DijkstraMap();

    void compute(Map<Distance> & distances,
                 const Map<unsigned int> & costs,
                 Vec2u start);
    void compute(Map<Distance> & distances,
                 const Map<unsigned int> & costs,
                 const std::vector<Vec2u> & start);
    void compute(Map<Distance> & distances,
                 const Map<unsigned int> & costs,
                 const std::vector<Goal> & start);
    void compute(Map<Distance> & distances_out,
                 const Map<unsigned int> & costs,
                 const Map<Distance> & distances_in);

    Map<Distance> compute(const Map<unsigned int> & costs, Vec2u start) {
      Map<Distance> distances;
      compute(distances, costs, start);
      return distances;
    }
    Map<Distance> compute(const Map<unsigned int> & costs, const std::vector<Vec2u> & start) {
      Map<Distance> distances;
      compute(distances, costs, start);
      return distances;
    }
    Map<Distance> compute(const Map<unsigned int> & costs, const std::vector<Goal> & start) {
      Map<Distance> distances;
      compute(distances, costs, start);
      return distances;
    }
    Map<Distance> compute(const Map<unsigned int> & costs, const Map<Distance> & distances_in) {
      Map<Distance> distances;
      compute(distances, costs, distances_in);
      return distances;
    }

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
      Distance distance = 0;
      // cost to enter this cell
      unsigned int cost = 0;
      // pointer to location in heap
      HeapNode * heap_node = nullptr;
    };

    GraphNode * graph_nodes = nullptr;
    HeapNode * heap_nodes = nullptr;
    Vec2u graph_size;
    size_t heap_size = 0;

    void init_graph(const Map<unsigned int> & costs);
    void init(const Map<unsigned int> & costs, Vec2u start);
    void init(const Map<unsigned int> & costs, const std::vector<Vec2u> & start);
    void init(const Map<unsigned int> & costs, const std::vector<Goal> & start);
    void init(const Map<unsigned int> & costs, const Map<Distance> & start);

    GraphNode * heap_pop();
    void heap_decrease(GraphNode * node, Distance distance);

    void do_dijkstra(Map<Distance> & distances);
  };
}

#endif
