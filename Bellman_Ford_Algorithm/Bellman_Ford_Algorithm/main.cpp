//
//  main.cpp
//  Bellman_Ford_Algorithm
//
//  Created by Jakub Koczwara on 15.10.2016.
//  Copyright © 2016 Jakub Koczwara. All rights reserved.
//

#include <iostream>
#include <assert.h>

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

void internal
initialize(int32 distance[], int32 parent[], int32 edges) {
    for (uint32 i = 0; i < edges; i++) {
        distance[i] = INT32_MAX / 2; // have to be smaller than max -> possible overflow
        parent[i] = -1;
    }
}

int32 internal
compute(Edge edges[], int32 start, int32 shortestPath[], int32 parent[], int32 size)
{
    initialize(shortestPath, parent, size);
    shortestPath[start] = 0;
    
    // Step 2: relax edges repeatedly
    for (int32 iteration = 0; iteration < size - 1; ++iteration)
    {
        for (int32 vert = 0; vert < size - 1; ++vert)
        {
            Edge e = edges[vert];
            if (shortestPath[e.to] > shortestPath[e.from] + e.weight) {
                shortestPath[e.to] = shortestPath[e.from] + e.weight;
                parent[e.to] = e.from;
            }
        }
    }
    
    // Step 3: check for negative-weight cycles
    for (int32 vert = 0; vert < size - 1; ++vert)
    {
        Edge e = edges[vert];
        if (shortestPath[e.to] > shortestPath[e.from] + e.weight) {
            return 0;
        }
    }
    
    return 1;
}

template <size_t rows, size_t cols>
int32 internal
compute(int32 (&edges)[rows][cols], int32 start, int32 shortestPath[], int32 parent[], int32 size)
{
    initialize(shortestPath, parent, size);
    
    shortestPath[start] = 0;
    
    for (int32 iteration = 0; iteration < size - 1; ++iteration)
    {
        for (int32 to = 0; to < cols; ++to)
        {
            for (int32 from = 0; from < rows; ++from)
            {
                if (edges[to][from] == NL) continue;
                
                
                if (shortestPath[to] > shortestPath[from] + edges[to][from]) {
                    shortestPath[to] = shortestPath[from] + edges[to][from];
                    parent[to] = from;
                }
            }
        }
    }
    
    for (int32 to = 0; to < cols; ++to)
    {
        for (int32 from = 0; from < rows; ++from)
        {
            if (edges[to][from] == NL) continue;
            
            
            if (shortestPath[to] > shortestPath[from] + edges[to][from]) {
                return 0;
            }
        }
    }
    
    return 1;
}

static int32 *
create_empty_array(int32 size) {
    int32 * array;
    
    array = (int32*) calloc(size, sizeof(int32));
    assert(array);
    
    return array;
}

int main(int argc, const char * argv[]) {
    
    Edge edges[8]= {
        // from, to, weight
        {0,  1,  10},
        {0,  5,  8},
        {5,  4,  1},
        {4,  1,  -4},
        {1,  3,  2},
        {3,  2,  -2},
        {2,  1,  1},
        {4,  3,  -1}};
    
    int32 size = sizeof(edges)/sizeof(Edge);
    
    int32 adjacency_matrix[6][6] = {
    //f   0   1   2   3    4     5
        {NL, NL, NL,  NL,  NL,  NL}, // 0 t
        {10, NL,  1,  NL,  -4,  NL}, // 1
        {NL, NL, NL,  -2,  NL,  NL}, // 2
        {NL,  2, NL,  NL,  -1,  NL}, // 3
        {NL, NL, NL,  NL,  NL,  1 }, // 4
        { 8, NL, NL,  NL,  NL,  NL}  // 5
    };
    
    int32* distance = create_empty_array(size);
    int32* parent = create_empty_array(size);
    
    int32 start = 0;
    
    //int32 noNegativeRoutes = compute(edges, start, distance, parent, size);
    int32 noNegativeRoutes = compute(adjacency_matrix, start, distance, parent, size);
    
    std::cout << "Bellman Ford returned " << noNegativeRoutes << "\n";
    if (noNegativeRoutes == 1)
    {
        for (int32 i = 1; i < 6; ++i)
        {
            std::cout << "Distance to " << i << " from " << parent[i] << " is " << distance[i] << "\n";
            
        }
    }
    
    return 0;
}

