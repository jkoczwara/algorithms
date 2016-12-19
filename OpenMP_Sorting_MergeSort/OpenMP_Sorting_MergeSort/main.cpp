//
//  main.cpp
//  OpenMP_Parallel
//
//  Created by Jakub Koczwara on 20.11.2016.
//  Copyright © 2016 Jakub Koczwara. All rights reserved.
//

#include <libiomp/omp.h>
#include <iostream>
#include <assert.h>
#include <sys/time.h>

#define internal static
#define local_persist static
#define global_variable static

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

global_variable int32 number_of_threads = 0;
global_variable int32 size_of_array = 1000000;

#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))

// ****************** ALGORITHM ***************************

// ****************** ONE THREAD ***************************

template<typename type>
internal void
merge(type *array, int32 firstEl, int32 midEl, int32 lastEl) {
    //    std::cout << firstEl << " " << midEl << " " << lastEl << std::endl;
    type *localArray = new type[lastEl - firstEl]();
    int32 index = 0;
    
    int32 firstProbe = firstEl;
    int32 secondProbe = midEl;
    
    while((firstProbe < midEl) && (secondProbe < lastEl)) {
        if (array[firstProbe] < array[secondProbe]) {
            localArray[index++] = array[firstProbe++];
        } else {
            localArray[index++] = array[secondProbe++];
        }
    }
    
    while(firstProbe < midEl) {
        localArray[index++] = array[firstProbe++];
    }
    
    while(secondProbe < lastEl) {
        localArray[index++] = array[secondProbe++];
    }

    int32 i = 0;
    index = firstEl;
    while(index < lastEl) {
        array[index++] = localArray[i++];
    }
    
    delete [] localArray;
}

template<typename type>
internal void
mergeSort(type *array, int32 firstEl, int32 lastEl) {
    if (lastEl - firstEl >= 2) {
        int32 midEl = (firstEl + lastEl) / 2;
        
        mergeSort(array, firstEl, midEl);
        mergeSort(array, midEl, lastEl);
        
        merge(array, firstEl, midEl, lastEl);
    }
}

template<typename type>
internal void
mergeSort(type *array, int32 size) {
    mergeSort(array, 0, size);
}

// ****************** PARALLEL ***************************

void insertion_sort(int32 arr[], int32 n)
{
    int32 i, key, j;
    for (i = 1; i < n; i++)
    {
        key = arr[i];
        j = i-1;
    
        while (j >= 0 && arr[j] > key)
        {
            arr[j+1] = arr[j];
            j--;
        }
        arr[j+1] = key;
    }
}

internal void
serial_mergeSort(int32 *array, int32 firstEl, int32 lastEl) {
    if (lastEl - firstEl >= 2) {
        int32 midEl = (firstEl + lastEl) / 2;
        
        if (lastEl - firstEl <= 4) {
            insertion_sort(array + firstEl, lastEl - firstEl);
            return;
        }
        serial_mergeSort(array, firstEl, midEl);
        serial_mergeSort(array, midEl, lastEl);
        merge(array, firstEl, midEl, lastEl);
    }
}

template<typename type>
internal void
mergeSortParallel(type *array, int32 firstEl, int32 lastEl) {
    if (lastEl - firstEl >= 2) {
        int32 midEl = (firstEl + lastEl) / 2;
        
        if ( lastEl - firstEl < 1024) {
            // do not spawn new threads
            serial_mergeSort(array, firstEl, lastEl);

        } else {

#pragma omp task firstprivate(firstEl, midEl)
            {mergeSortParallel(array, firstEl, midEl);}
#pragma omp task firstprivate(lastEl, midEl)
            {mergeSortParallel(array, midEl, lastEl);}
#pragma omp taskwait
            merge(array, firstEl, midEl, lastEl);
            
        }
    }
}

template<typename type>
internal void
mergeSortParallel(type *array, int32 size) {
#pragma omp parallel num_threads(number_of_threads)
#pragma omp master
    mergeSortParallel(array, 0, size);
}


// 0 1 2 3 4 5
// ---
//   ---
//     ---

// -- -- --
//  --  -- --
// -- -- --

internal void
bubbleSort(int * array, int size) {
    for(int32 i = 0; i < size; i++)
    {
        int first = i % 2;
        
#pragma omp parallel for default(none) shared(array, first, size)
        for(int32 j = first; j < size - 1; j += 2)
        {
            if (array[j] > array[j + 1])
            {
                std::swap(array[j], array[j + 1]);
            }
        }
    }
    
}

internal void
bubbleSortSeq(int32* array, int32 size) {
    for(int32 i = 0; i < size; i++)
    {
        bool stop = true;
        for (int32 j = 0; j < size - 1; j++)
        {
            if (array[j] > array[j + 1])
            {
                std::swap(array[j], array[j + 1]);
                stop = false;
            }
        }
        if(stop)
            break;
    }
    
}


template <typename F, typename A1>
internal void
make_computation_and_print_elapsed_time(A1 arg1[], A1 arg2, F (*f)(A1[], A1)) {
    std::cout << "Computing...\n";
    
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC_RAW, &start);
    
    (*f)(arg1, arg2);
    
    clock_gettime(CLOCK_MONOTONIC_RAW, &end);
    uint64_t delta_us = (end.tv_sec - start.tv_sec) * 1000 + (end.tv_nsec - start.tv_nsec) / 1000000;
    
    std::cout << "Sorted in " << delta_us << " [ms].\n";
}

template<typename type>
internal void
print_array(type const array[], int32 size) {
    for(int i = 0; i < size; ++i) {
        std::cout << array[i] << " ";
    }
    std::cout<< std::endl;
}

template<typename type>
internal bool check_if_sorted(type const array[], int32 size) {
    for (int i = 0; i < size - 1; i++) {
        if (array[i] > array[i+1]) {
            return false;
        }
    }
    return true;
}


internal void
init() {
#pragma omp parallel 
    {
        number_of_threads = omp_get_num_threads();
    }
    assert(number_of_threads > 0);
    std::cout << "Number of threads: " << number_of_threads << std::endl;
    omp_set_nested(1);
}



// ****************** MAIN ***************************

int main(int argc, const char * argv[]) {
    init();
    
    std::cout << "Generating random " << size_of_array << " numbers...\n";
    int32 *array_to_sort = new int32[size_of_array];
    for (int i = 0; i < size_of_array; i++) {
        array_to_sort[i] = rand() % 100;
    }
    int32 *array_to_sort_parallel = new int32[size_of_array];
    std::copy(&array_to_sort[0], &array_to_sort[0] + size_of_array, &array_to_sort_parallel[0]);
    
    
    make_computation_and_print_elapsed_time(array_to_sort, size_of_array, mergeSort<int32>);
    make_computation_and_print_elapsed_time(array_to_sort_parallel, size_of_array, mergeSortParallel<int32>);

//    make_computation_and_print_elapsed_time(array_to_sort, size_of_array, bubbleSortSeq);
//    make_computation_and_print_elapsed_time(array_to_sort_parallel, size_of_array, bubbleSort);

    //print_array(array_to_sort_parallel, size_of_array);
    assert(check_if_sorted(array_to_sort, size_of_array));
    assert(check_if_sorted(array_to_sort_parallel, size_of_array));
    
    return 0;
}
