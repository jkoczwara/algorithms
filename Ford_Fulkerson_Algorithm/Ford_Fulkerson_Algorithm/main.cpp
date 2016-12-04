//
//  main.cpp
//  Ford_Fulkerson_Algorithm
//
//  Created by Jakub Koczwara on 07.11.2016.
//  Copyright © 2016 Jakub Koczwara. All rights reserved.
//


#include <iostream>
#include <assert.h>
#include <queue>

#define internal static
#define local_persist static
#define global_variable static

#define NL INT32_MIN

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef int32_t bool32;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef float real32;
typedef double real64;

#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))

struct Edge {
    int32 from;
    int32 to;
    int32 weight;
    
    friend
    std::ostream & operator<<(std::ostream & out, Edge const & e) {
        return out << e.from << " " << e.to << " " << e.weight << std::endl;
    }
};

template<typename type>
internal void
print_array(type const array[], int32 const size) {
    for(int i = 0; i < size; ++i) {
        std::cout << array[i] << " ";
    }
    std::cout<< std::endl;
}

// ****************** LIST OF EDGES ***************************

internal std::vector<Edge>
get_edges_from(int32 from, std::vector<Edge> edges) {
    std::vector<Edge> _ret;
    
    for (auto &edge: edges) {
        if(from == edge.from) {
            _ret.push_back(edge);
        }
    }
    
    return _ret;
}

internal int32
bfs(std::vector<Edge> edges, int32 const start, int32 const target, Edge parent[], int32 no_edges) {
    int32 size = no_edges;
    int32 visited[size];
    memset(parent, 0, size * sizeof(Edge));
    memset(visited, 0, size * sizeof(int32));
    
    std::queue <int32> queue;
    queue.push(start);
    visited[start] = 1;
    
    while (!queue.empty()) {
        int32 current = queue.front();
        queue.pop();

        for (auto &edge: get_edges_from(current, edges)) {

            if (visited[edge.to] == 0 && edge.weight > 0) {
                queue.push(edge.to);
                parent[edge.to] = edge;
                visited[edge.to] = 1;
            }
        }
    }
    
    return visited[target] == 1;
}

internal void
update_edge_in_graph(std::vector<Edge> &graph, Edge e, int32 value) {
    for (auto &edge: graph) {
        if (edge.from == e.from && edge.to == e.to) {
            edge.weight -= value;
            assert(edge.weight >= 0);
            
            return;
        }
    }
    
    assert(false);
}

internal void
update_reverse_edge_in_graph(std::vector<Edge> &graph, Edge e, int32 value) {
    for (auto &edge: graph) {
        if (edge.from == e.to && edge.to == e.from) {
            edge.weight += value;
            
            return;
        }
    }

    Edge reverse_edge;
    reverse_edge.from = e.to;
    reverse_edge.to = e.from;
    reverse_edge.weight = value;
    
    graph.push_back(reverse_edge);
}

template <size_t number_of_edges>
internal int32
compute(Edge const(&edges)[number_of_edges], int32 const start, int32 const sink, int32 const no_nodes)
{
    // initialize
    int32 size = no_nodes;
    int32 max_flow = 0;
    Edge parent[size];
    
    std::vector<Edge> residual_graph(std::begin(edges), std::end(edges));
    
    // compute
    while(bfs(residual_graph, start, sink, parent, no_nodes)) {
        int32 path_flow = INT32_MAX;
        
        // find minimum residual path flow value (aka. max available flow through the path)
        for (Edge v = parent[sink]; ; v = parent[v.from]) {
            path_flow = std::min(path_flow, v.weight);
            if (v.from == start) {
                break;
            }
        }
        
        for (Edge v = parent[sink]; ; v = parent[v.from]) {
            update_edge_in_graph(residual_graph, v, path_flow);
            update_reverse_edge_in_graph(residual_graph, v, path_flow);
            
            if (v.from == start) {
                break;
            }
        }
        
        max_flow += path_flow;
    }

    
    return max_flow;
}

// ****************** ADJACENCY MATRIX ***************************

template <size_t rows, size_t cols>
internal int32
bfs(int32 const (&graph)[rows][cols], int32 const start, int32 const target, int32 parent[]) {
    int32 size = rows;
    int32 visited[size];
    memset(parent, 0, size * sizeof(int32));
    memset(visited, 0, size * sizeof(int32));
    
    std::queue <int32> queue;
    queue.push(start);
    visited[start] = 1;
    
    while (!queue.empty()) {
        int32 current = queue.front();
        queue.pop();
        
        for (int32 v = 0; v < size; ++v) {
            if (visited[v] == 0 && graph[current][v] > 0) {
                queue.push(v);
                parent[v] = current;
                visited[v] = 1;
            }
        }
    }
    
    return visited[target] == 1;
}

template <size_t rows, size_t cols>
internal int32
compute(int32 const(&graph)[rows][cols], int32 const start, int32 const sink)
{
    // initialize
    int32 size = rows;
    int32 max_flow = 0;
    int32 parent[size];
    
    int32 residual_graph[rows][cols];
    std::copy(&graph[0][0], &graph[0][0] + size * size, &residual_graph[0][0]);
    
    // compute
    while(bfs(residual_graph, start, sink, parent)) {
        int32 path_flow = INT32_MAX;
        
        // find minimum residual path flow value (aka. max available flow through the path)
        for (int32 v = sink ; v != start; v = parent[v]) {
            int32 u = parent[v];
            path_flow = std::min(path_flow, residual_graph[u][v]);
        }
        
        for (int32 v = sink ; v != start; v = parent[v]) {
            int32 u = parent[v];
            
            // update residual path by substracting computed path flow
            residual_graph[u][v] -= path_flow;
            
            // add reverse paths
            residual_graph[v][u] += path_flow;
        }
        
        max_flow += path_flow;
    }
    
    return max_flow;
}

int main(int argc, const char * argv[]) {
    Edge edges[10]= {
    // from, to, weight
        {0,  1,  16},
        {0,  2,  13},
        {1,  2,  10},
        {1,  3,  12},
        {2,  1,  4},
        {2,  4,  14},
        {3,  2,  9},
        {3,  5,  20},
        {4,  3,  7},
        {4,  5,  4}};
    
    int32 graph[6][6] =
    // column -> from, row -> to
    //to   0   1   2   3   4   5     from
         {{0, 16, 13,  0,  0,  0},   // 0
          {0,  0, 10, 12,  0,  0},   // 1
          {0,  4,  0,  0, 14,  0},   // 2
          {0,  0,  9,  0,  0, 20},   // 3
          {0,  0,  0,  7,  0,  4},   // 4
          {0,  0,  0,  0,  0,  0}    // 5
    };
    
    int32 start = 0;
    int32 sink = 5;
    assert(start != sink);

    clock_t begin = clock();
    int32 flow_graph = compute(graph, start, sink);
    clock_t end = clock();
    double elapsed_clocks = double(end - begin);
    std::cout << "Ford Fuklerson - graph: max computed flow " << flow_graph << " in " << elapsed_clocks << " clocks \n";
    
    clock_t begin_edges = clock();
    int32 flow_edges = compute(edges, start, sink, 6);
    clock_t end_edges = clock();
    double elapsed_clocks_edges = double(end_edges - begin_edges);
    std::cout << "Ford Fuklerson - edges: max computed flow " << flow_edges << " in " << elapsed_clocks_edges << " clocks \n";
    
    assert(flow_edges == flow_graph);
    
    return 0;
}

