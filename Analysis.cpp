#include <iostream>
#include <vector>
#include <queue>
#include <limits>
#include <chrono>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <boost/graph/bellman_ford_shortest_paths.hpp>
#include <boost/heap/fibonacci_heap.hpp>

using namespace std;
using namespace boost;

// Define a graph using the Boost Graph Library
typedef adjacency_list<vecS, vecS, directedS,
    no_property, property<edge_weight_t, int>> Graph;
typedef graph_traits<Graph>::vertex_descriptor Vertex;
typedef graph_traits<Graph>::edge_descriptor Edge;
typedef property_map<Graph, edge_weight_t>::type WeightMap;

// Define a Fibonacci Heap as the priority queue for A*
typedef fibonacci_heap<Vertex, compare<function<bool(Vertex, Vertex)>>> FibonacciHeap;

// A* heuristic function (assuming Euclidean distance)
struct AStarHeuristic {
    Vertex target;
    WeightMap weights;

    AStarHeuristic(Vertex target, WeightMap weights) : target(target), weights(weights) {}

    int operator()(Vertex v) const {
        return weights[target] - weights[v];
    }
};

// Function to perform Dijkstra's algorithm
void dijkstra(const Graph& g, Vertex source, WeightMap& distances) {
    dijkstra_shortest_paths(g, source, distance_map(distances));
}

// Function to perform Bellman-Ford algorithm
bool bellman_ford(const Graph& g, Vertex source, WeightMap& distances) {
    return bellman_ford_shortest_paths(g, num_vertices(g), distance_map(distances).distance_map(distances));
}

// Function to perform A* algorithm
void astar(const Graph& g, Vertex source, Vertex target, WeightMap& distances) {
    FibonacciHeap pq;
    vector<FibonacciHeap::handle_type> handles(num_vertices(g));

    for (Vertex v : vertices(g)) {
        handles[v] = pq.push(v);
    }

    distances[source] = 0;
    pq.increase(handles[source], AStarHeuristic(target, distances)(source));

    while (!pq.empty()) {
        Vertex u = pq.top();
        pq.pop();

        for (const Edge& e : out_edges(u, g)) {
            Vertex v = target(e);
            int weight = get(weights, e);
            if (distances[u] + weight < distances[v]) {
                distances[v] = distances[u] + weight;
                pq.increase(handles[v], AStarHeuristic(target, distances)(v));
            }
        }
    }
}

int main() {
    Graph g(5);  // Create a sample graph with 5 vertices

    // Define the edges and weights
    vector<pair<int, int>> edges = { {0, 1}, {0, 2}, {1, 3}, {2, 3}, {3, 4} };
    vector<int> weights = { 2, 1, 3, 1, 2 };

    WeightMap distances(num_vertices(g), numeric_limits<int>::max());

    // Add edges and weights to the graph
    for (size_t i = 0; i < edges.size(); ++i) {
        Edge e;
        tie(e, ignore) = add_edge(edges[i].first, edges[i].second, g);
        put(weights, e, weights[i]);
    }

    Vertex source = 0;
    Vertex target = 4;

    // Measure the time for each algorithm
    auto start_time = chrono::high_resolution_clock::now();
    dijkstra(g, source, distances);
    auto end_time = chrono::high_resolution_clock::now();
    auto dijkstra_time = chrono::duration_cast<chrono::microseconds>(end_time - start_time).count();

    start_time = chrono::high_resolution_clock::now();
    bool bf_result = bellman_ford(g, source, distances);
    end_time = chrono::high_resolution_clock::now();
    auto bellman_ford_time = chrono::duration_cast<chrono::microseconds>(end_time - start_time).count();

    start_time = chrono::high_resolution_clock::now();
    astar(g, source, target, distances);
    end_time = chrono::high_resolution_clock::now();
    auto astar_time = chrono::duration_cast<chrono::microseconds>(end_time - start_time).count();

    // Print results
    cout << "Dijkstra's Shortest Path: ";
    if (distances[target] != numeric_limits<int>::max()) {
        cout << distances[target] << " (Time: " << dijkstra_time << " microseconds)" << endl;
    } else {
        cout << "No path found" << endl;
    }

    cout << "Bellman-Ford Shortest Path: ";
    if (bf_result) {
        cout << distances[target] << " (Time: " << bellman_ford_time << " microseconds)" << endl;
    } else {
        cout << "Negative cycle detected" << endl;
    }

    cout << "A* Shortest Path: ";
    if (distances[target] != numeric_limits<int>::max()) {
        cout << distances[target] << " (Time: " << astar_time << " microseconds)" << endl;
    } else {
        cout << "No path found" << endl;
    }

    return 0;
}
