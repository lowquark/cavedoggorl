
#include "Dijkstra.hpp"

#include <cassert>
#include <utility>

namespace rf {
  DijkstraMap::~DijkstraMap() {
    delete [] graph_nodes;
  }

  void DijkstraMap::compute(Map<Distance> & distances,
                            const Map<unsigned int> & costs,
                            Vec2u start) {
    init(costs, start);
    do_dijkstra(distances);
  }
  void DijkstraMap::compute(Map<Distance> & distances,
                            const Map<unsigned int> & costs,
                            const std::vector<Vec2u> & start) {
    init(costs, start);
    do_dijkstra(distances);
  }
  void DijkstraMap::compute(Map<Distance> & distances,
                            const Map<unsigned int> & costs,
                            const std::vector<Goal> & start) {
    init(costs, start);
    do_dijkstra(distances);
  }
  void DijkstraMap::compute(Map<Distance> & distances_out,
                            const Map<unsigned int> & costs,
                            const Map<Distance> & distances_in) {
    init(costs, distances_in);
    do_dijkstra(distances_out);
  }

  void DijkstraMap::test_heap() {
    rf::DijkstraMap dm;

    rf::Map<unsigned int> costs(Vec2u(3, 3));
    costs.fill(1);
    dm.init(costs, Vec2u(1, 1));

    assert(dm.graph_size == Vec2u(3, 3));
    assert(dm.heap_size == 9);

    assert(dm.heap_nodes[0]->distance == 0);
    for(int i = 1 ; i < 9 ; i ++) {
      assert(dm.heap_nodes[i]->distance == infinity);
    }

    // heap_nodes[0,1,4] should have had their `graph_node` cycled from percolation
    assert(dm.heap_nodes[0] == &dm.graph_nodes[4]);
    assert(dm.heap_nodes[1] == &dm.graph_nodes[0]);
    assert(dm.heap_nodes[2] == &dm.graph_nodes[2]);
    assert(dm.heap_nodes[3] == &dm.graph_nodes[3]);
    assert(dm.heap_nodes[4] == &dm.graph_nodes[1]);
    assert(dm.heap_nodes[5] == &dm.graph_nodes[5]);
    assert(dm.heap_nodes[6] == &dm.graph_nodes[6]);
    assert(dm.heap_nodes[7] == &dm.graph_nodes[7]);
    assert(dm.heap_nodes[8] == &dm.graph_nodes[8]);

    GraphNode * node = dm.heap_pop();
    assert(dm.heap_size == 8);
    assert(node == &dm.graph_nodes[4]);
    assert(node->distance == 0);

    // the tree should be the same, except the top node is now the old last node
    assert(dm.heap_nodes[0] == &dm.graph_nodes[8]);
    assert(dm.heap_nodes[1] == &dm.graph_nodes[0]);
    assert(dm.heap_nodes[2] == &dm.graph_nodes[2]);
    assert(dm.heap_nodes[3] == &dm.graph_nodes[3]);
    assert(dm.heap_nodes[4] == &dm.graph_nodes[1]);
    assert(dm.heap_nodes[5] == &dm.graph_nodes[5]);
    assert(dm.heap_nodes[6] == &dm.graph_nodes[6]);
    assert(dm.heap_nodes[7] == &dm.graph_nodes[7]);

    dm.heap_decrease(dm.heap_nodes[7], 4545);
    assert(dm.heap_nodes[0]->distance == 4545);
    assert(dm.heap_nodes[0] == &dm.graph_nodes[7]);

    dm.heap_decrease(dm.heap_nodes[7], 3434);
    assert(dm.heap_nodes[0]->distance == 3434);
    assert(dm.heap_nodes[0] == &dm.graph_nodes[3]);

    dm.heap_decrease(dm.heap_nodes[7], 2323);
    assert(dm.heap_nodes[0]->distance == 2323);
    assert(dm.heap_nodes[0] == &dm.graph_nodes[0]);

    dm.heap_decrease(dm.heap_nodes[7], 1212);
    assert(dm.heap_nodes[0]->distance == 1212);
    assert(dm.heap_nodes[0] == &dm.graph_nodes[8]);

    dm.heap_decrease(dm.heap_nodes[7],  101);
    assert(dm.heap_nodes[0]->distance ==  101);
    assert(dm.heap_nodes[0] == &dm.graph_nodes[7]);

    assert(dm.heap_pop() == &dm.graph_nodes[7]);
    assert(dm.heap_size == 7);
    assert(dm.heap_pop() == &dm.graph_nodes[8]);
    assert(dm.heap_size == 6);
    assert(dm.heap_pop() == &dm.graph_nodes[0]);
    assert(dm.heap_size == 5);
    assert(dm.heap_pop() == &dm.graph_nodes[3]);
    assert(dm.heap_size == 4);
  }
  void DijkstraMap::test() {
    test_heap();

    rf::DijkstraMap dm;

    const Vec2u size(10, 10);

    rf::Map<unsigned int> costs(size);
    for(unsigned int y = 0 ; y < costs.size().x ; y ++) {
      for(unsigned int x = 0 ; x < costs.size().x ; x ++) {
        if(x == 5) {
          costs[Vec2u(x, y)] = 20;
        } else {
          costs[Vec2u(x, y)] = 1;
        }
      }
    }

    auto map = dm.compute(costs, Vec2u(1, 1));

    for(unsigned int y = 0 ; y < map.size().x ; y ++) {
      for(unsigned int x = 0 ; x < map.size().x ; x ++) {
        printf("%4d ", map[Vec2u(x, y)]);
      }
      printf("\n");
    }

    map = dm.compute(costs, { Vec2u(1, 1), Vec2u(2, 2) });

    printf("---\n");
    for(unsigned int y = 0 ; y < map.size().x ; y ++) {
      for(unsigned int x = 0 ; x < map.size().x ; x ++) {
        printf("%4d ", map[Vec2u(x, y)]);
      }
      printf("\n");
    }

    map = dm.compute(costs, { std::make_pair(Vec2u(1, 1), -5),
                              std::make_pair(Vec2u(2, 2), -2),
                              std::make_pair(Vec2u(3, 3), 0) });

    printf("---\n");
    for(unsigned int y = 0 ; y < map.size().x ; y ++) {
      for(unsigned int x = 0 ; x < map.size().x ; x ++) {
        printf("%4d ", map[Vec2u(x, y)]);
      }
      printf("\n");
    }

    // multiply each by -1.2
    for(unsigned int y = 0 ; y < map.size().x ; y ++) {
      for(unsigned int x = 0 ; x < map.size().x ; x ++) {
        map[Vec2u(x, y)] = -(map[Vec2u(x, y)] * 6 / 5);
      }
    }

    dm.compute(map, costs, map);

    printf("---\n");
    for(unsigned int y = 0 ; y < map.size().x ; y ++) {
      for(unsigned int x = 0 ; x < map.size().x ; x ++) {
        printf("%4d ", map[Vec2u(x, y)]);
      }
      printf("\n");
    }
  }

  void DijkstraMap::init_graph(const Map<unsigned int> & costs) {
    Vec2u size = costs.size();

    assert(size.x != 0);
    assert(size.y != 0);

    unsigned int node_num = size.x * size.y;

    // reallocate if we need more space
    if(node_num > graph_size.x * graph_size.y) {
      delete [] graph_nodes;
      graph_nodes = new GraphNode [node_num];
      delete [] heap_nodes;
      heap_nodes = new HeapNode [node_num];
    }
    graph_size = size;
    heap_size = node_num;

    unsigned int idx = 0;
    for(unsigned int y = 0 ; y < size.x ; y ++) {
      for(unsigned int x = 0 ; x < size.x ; x ++) {
        auto & node = graph_nodes[idx];

        // neighbor 0: x + 1, y
        if(x < size.x - 1) {
          node.neighbors[0] = &graph_nodes[(x + 1) + (y)*size.x];
        } else {
          node.neighbors[0] = nullptr;
        }

        // neighbor 1: x + 1, y + 1
        if(x < size.x - 1 && y < size.y - 1) {
          node.neighbors[1] = &graph_nodes[(x + 1) + (y + 1)*size.x];
        } else {
          node.neighbors[1] = nullptr;
        }

        // neighbor 2: x, y + 1
        if(y < size.y - 1) {
          node.neighbors[2] = &graph_nodes[(x) + (y + 1)*size.x];
        } else {
          node.neighbors[2] = nullptr;
        }

        // neighbor 3: x - 1, y + 1
        if(x > 0 && y < size.y - 1) {
          node.neighbors[3] = &graph_nodes[(x - 1) + (y + 1)*size.x];
        } else {
          node.neighbors[3] = nullptr;
        }

        // neighbor 4: x - 1, y
        if(x > 0) {
          node.neighbors[4] = &graph_nodes[(x - 1) + (y)*size.x];
        } else {
          node.neighbors[4] = nullptr;
        }

        // neighbor 5: x - 1, y - 1
        if(x > 0 && y > 0) {
          node.neighbors[5] = &graph_nodes[(x - 1) + (y - 1)*size.x];
        } else {
          node.neighbors[5] = nullptr;
        }

        // neighbor 6: x, y - 1
        if(y > 0) {
          node.neighbors[6] = &graph_nodes[(x) + (y - 1)*size.x];
        } else {
          node.neighbors[6] = nullptr;
        }

        // neighbor 7: x + 1, y - 1
        if(x < size.x - 1 && y > 0) {
          node.neighbors[7] = &graph_nodes[(x + 1) + (y - 1)*size.x];
        } else {
          node.neighbors[7] = nullptr;
        }

        node.distance = infinity;
        node.cost = costs[Vec2u(x, y)];
        node.heap_node = &heap_nodes[idx];
        heap_nodes[idx] = &graph_nodes[idx];
        idx ++;
      }
    }
  }
  void DijkstraMap::init(const Map<unsigned int> & costs, Vec2u start) {
    assert(costs.valid(start));
    init_graph(costs);

    // Update the starting node's distance
    GraphNode * start_node = &graph_nodes[(start.x) + (start.y)*costs.size().x];
    // Update it in the heap
    heap_decrease(start_node, 0);
  }
  void DijkstraMap::init(const Map<unsigned int> & costs, const std::vector<Vec2u> & start) {
    for(auto & p : start) {
      assert(costs.valid(p));
    }
    init_graph(costs);

    for(auto & p : start) {
      GraphNode * start_node = &graph_nodes[(p.x) + (p.y)*costs.size().x];
      heap_decrease(start_node, 0);
    }
  }
  void DijkstraMap::init(const Map<unsigned int> & costs, const std::vector<Goal> & start) {
    for(auto & p : start) {
      assert(costs.valid(p.first));
    }
    init_graph(costs);

    for(auto & p : start) {
      GraphNode * start_node = &graph_nodes[(p.first.x) + (p.first.y)*costs.size().x];
      heap_decrease(start_node, p.second);
    }
  }
  void DijkstraMap::init(const Map<unsigned int> & costs, const Map<Distance> & start) {
    assert(costs.size() == start.size());
    init_graph(costs);

    unsigned int idx = 0;
    for(unsigned int y = 0 ; y < graph_size.x ; y ++) {
      for(unsigned int x = 0 ; x < graph_size.x ; x ++) {
        GraphNode * node = &graph_nodes[idx];
        heap_decrease(node, start[Vec2u(x, y)]);
        idx ++;
      }
    }
  }

  DijkstraMap::GraphNode * DijkstraMap::heap_pop() {
    assert(heap_size > 0);
    GraphNode * top = heap_nodes[0];
    top->heap_node = nullptr;

    heap_size --;

    if(heap_size != 0) {
      // bring last element to top
      heap_nodes[0] = heap_nodes[heap_size];
      heap_nodes[heap_size] = nullptr;;

      // percolate it down
      unsigned int index = 0;

      while(true) {
        GraphNode * node = heap_nodes[index];
        unsigned int child_a_index = index*2 + 1;
        unsigned int child_b_index = index*2 + 2;

        if(child_a_index >= heap_size && child_b_index >= heap_size) {
          // we have reached a leaf
          break;
        } else if(child_b_index >= heap_size) {
          // child a is valid, child b is past the end
          GraphNode * child_a = heap_nodes[child_a_index];
          if(child_a->distance < node->distance) {
            // swap in heap
            std::swap(heap_nodes[index], heap_nodes[child_a_index]);
            // swap references to heap
            std::swap(node->heap_node, child_a->heap_node);
            index = child_a_index;
            // this particular case can only occur at the bottom of a tree
            break;
          } else {
            // no swap necessary, done swapping
            break;
          }
        } else {
          // both are valid
          GraphNode * child_a = heap_nodes[child_a_index];
          GraphNode * child_b = heap_nodes[child_b_index];

          // check if either child has a smaller value
          if(child_a->distance < node->distance || child_b->distance < node->distance) {
            // swap with the smallest child
            if(child_a->distance < child_b->distance) {
              // swap in heap
              std::swap(heap_nodes[index], heap_nodes[child_a_index]);
              // swap references to heap
              std::swap(node->heap_node, child_a->heap_node);
              index = child_a_index;
            } else {
              // swap in heap
              std::swap(heap_nodes[index], heap_nodes[child_b_index]);
              // swap references to heap
              std::swap(node->heap_node, child_b->heap_node);
              index = child_b_index;
            }
          } else {
            // neither child is smaller, so we're done swapping
            break;
          }
        }
      }
    }

    return top;
  }
  void DijkstraMap::heap_decrease(GraphNode * node, Distance new_distance) {
    // the value must decrease
    assert(new_distance <= node->distance);

    assert(node);
    assert(node->heap_node);

    // determine index of node
    unsigned int index = node->heap_node - heap_nodes;

    assert(index >= 0);
    assert(index < heap_size);

    // update value
    node->distance = new_distance;

    // heap is possibly invalid, percolate up
    while(index != 0) {
      unsigned int parent_index = (index - 1)/2;

      // compare with parent
      //GraphNode * node = heap_nodes[index]; // this will never actually change
      GraphNode * parent = heap_nodes[parent_index];

      //printf("index: %u\n", index);
      //printf("parent_index: %u\n", parent_index);
      //printf("d(index): %u\n", node->distance);
      //printf("d(parent_index): %u\n", parent->distance);

      // swap if smaller
      if(node->distance < parent->distance) {
        // swap in heap
        std::swap(heap_nodes[index], heap_nodes[parent_index]);
        // swap references to heap
        std::swap(node->heap_node, parent->heap_node);

        // continue with parent_index (now our same node)
        index = parent_index;
      } else {
        // no more swapping
        break;
      }
    }
  }

  void DijkstraMap::do_dijkstra(Map<Distance> & distances) {
    while(heap_size > 0) {
      GraphNode * closest_node = heap_pop();

      for(int i = 0 ; i < 8 ; i ++) {
        GraphNode * neighbor = closest_node->neighbors[i];

        if(neighbor && neighbor->heap_node != nullptr) {
          Distance possible_distance = closest_node->distance + neighbor->cost;

          // possible_distance >= closest_node->distance verifies that 
          // the result did not wrap around
          if(possible_distance >= closest_node->distance &&
             possible_distance < neighbor->distance) {
            heap_decrease(neighbor, possible_distance);
          }
        }
      }
    }

    distances.resize(graph_size);

    unsigned int idx = 0;
    for(unsigned int y = 0 ; y < graph_size.y ; y ++) {
      for(unsigned int x = 0 ; x < graph_size.x ; x ++) {
        auto & node = graph_nodes[idx];
        distances[Vec2u(x, y)] = node.distance;
        idx ++;
      }
    }
  }
}

