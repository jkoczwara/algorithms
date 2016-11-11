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
};

template<typename type>
internal void
print_array(type array[], int32 size) {
    for(int i = 0; i < size; ++i) {
        std::cout << array[i] << " ";
    }
    std::cout<< std::endl;
}

template <size_t rows, size_t cols>
internal int32
bfs(int32 (&graph)[rows][cols], int32 start, int32 target, int32 parent[]) {
    int32 size = rows;
    int32 visited[size];
    memset(parent, 0, size * sizeof(int32));
    memset(visited, 0, size * sizeof(int32));
    
    std::queue <int32> queue;
    queue.push(start);
    visited[start] = 1;
    parent[start] = -1;
    
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

internal int32
compute(Edge edges[], int32 source, int32 sink, int32 size)
{
    
    return 0;
}

template <size_t rows, size_t cols>
internal int32
compute(int32 (&graph)[rows][cols], int32 start, int32 sink)
{
    // initialize
    int32 size = rows;
    int32 max_flow = 0;
    int32 parent[size];
    
    int32 residual_graph[rows][cols];
    std::copy(&graph[0][0], &graph[0][0] + size * size, &residual_graph[0][0]);
    
    // compute
    while( bfs(residual_graph, start, sink, parent) ) {
        int32 path_flow = INT32_MAX;
        for (int32 v = sink ; v != start; v = parent[v]) {
            int32 u = parent[v];
            path_flow = std::min(path_flow, residual_graph[u][v]);
        }
        
        for (int32 v = sink ; v != start; v = parent[v]) {
            int32 u = parent[v];
            residual_graph[u][v] -= path_flow;
            residual_graph[v][u] += path_flow;
        }
        
        max_flow += path_flow;
    }
    
    return max_flow;
}

int main(int argc, const char * argv[]) {
    
    Edge graph_edges[8]= {
        // from, to, weight
        {0,  1,  10},
        {0,  5,  8},
        {5,  4,  1},
        {4,  1,  4},
        {1,  3,  2},
        {3,  2,  2},
        {2,  1,  1},
        {4,  3,  1}};
    
    int32 graph_matrix[6][6] = {
    //f   0   1   2   3    4     5
        {NL, NL, NL,  NL,  NL,  NL}, // 0 t
        {10, NL,  1,  NL,   4,  NL}, // 1
        {NL, NL, NL,   2,  NL,  NL}, // 2
        {NL,  2, NL,  NL,   1,  NL}, // 3
        {NL, NL, NL,  NL,  NL,  1 }, // 4
        { 8, NL, NL,  NL,  NL,  NL}  // 5
    };
    
    int32 graph[6][6] =
      { {0, 16, 13, 0, 0, 0},
        {0, 0, 10, 12, 0, 0},
        {0, 4, 0, 0, 14, 0},
        {0, 0, 9, 0, 0, 20},
        {0, 0, 0, 7, 0, 4},
        {0, 0, 0, 0, 0, 0}
    };
    
    int32 size = sizeof(graph_edges)/sizeof(Edge);
    
    int32 start = 0;
    int32 sink = 5;
    assert(sink <= size);

    int32 flow = compute(graph, start, sink);
    std::cout << "Ford Fuklerson: max computed flow " << flow << "\n";
    
    return 0;
}

