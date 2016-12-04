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

#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))

inline internal double
func(double x) {
    return 1 / (1 + x * x);
}

internal double
compute(double dx) {
    double sum = 0.0;
    double l =  1.0 / dx;
    
    for (int32 x = 0; x < l; ++x) {
        sum += func(x * dx) * dx;
    }
    
    return 4 * sum;
}

internal double
compute_with_pragma_on_for(double const dx) {
    double sum = 0.0;
    double l = 1 / dx;
    
#pragma omp parallel for reduction(+:sum)
    for (int32 x = 0; x < l; ++x) {
        sum += func(x * dx) * dx;
    }
    
    return 4 * sum;
}

internal double
compute_with_array_without_private_subsum(double dx) {
    double *array = new double[number_of_threads]();
    double l = 1.0 / dx;
    int32 x, current_thread;
    
#pragma omp parallel private(x, current_thread) shared(l, dx, array, number_of_threads) default(none)
    {
        current_thread = omp_get_thread_num();
        
        for (x = current_thread; x < l ; x += number_of_threads) {
            array[current_thread] += func(x * dx) * dx;
        }
    }
    
    double _ret_sum = 0.0;
    for (int32 i = 0; i < number_of_threads; ++i) {
        _ret_sum += array[i];
    }
    
    return 4 * _ret_sum;
}

internal double
compute_with_array_private_subsum(double dx) {
    double *array = new double[number_of_threads]();
    double sum;
    double l = 1.0 / dx;
    int32 x, current_thread;
    
#pragma omp parallel private(x, sum, current_thread) shared(l, dx, array, number_of_threads) default(none)
    {
        current_thread = omp_get_thread_num();
        sum = 0.0;
        
        for (x = current_thread; x < l ; x += number_of_threads) {
            sum += func(x * dx) * dx;
        }
        
        array[current_thread] = sum;
    }
    
    double _ret_sum = 0.0;
    for (int32 i = 0; i < number_of_threads; ++i) {
        _ret_sum += array[i];
    }
    
    return 4 * _ret_sum;
}

internal double
compute_with_array_private_subsum2(double dx) {
    double *array = new double[number_of_threads]();
    double sum;
    double l = 1.0 / dx;
    int32 x, current_thread;
    int32 start, end;
#pragma omp parallel private(x, sum, current_thread, start, end) shared(l, dx, array, number_of_threads) default(none)
    {
        current_thread = omp_get_thread_num();
        sum = 0.0;
        start = current_thread * l / number_of_threads;
        end = start + l / number_of_threads;
        for (x = start; x < end ; x += 1) {
            sum += func(x * dx) * dx;
        }
        
        array[current_thread] = sum;
    }
    
    double _ret_sum = 0.0;
    for (int32 i = 0; i < number_of_threads; ++i) {
        _ret_sum += array[i];
    }
    
    return 4 * _ret_sum;
}


// ****************** MONTE CARLO ***************************

internal double
count_points_in_circle_single_thread(double const points){
    std::srand((uint32)std::time(0));
    
    int32 count_inside = 0;
    double x, y;
    
    for (int i = 0 ; i < points ; i++){
        
        x = rand() / (double)RAND_MAX;
        y = rand() / (double)RAND_MAX;
        
        if( x * x + y * y < 1)  {
            count_inside++;
        }
    }
    
    return (count_inside / points) * 4;
}

internal double
count_points_in_circle_threads_on_loop(double const points){
    int32 count_inside = 0;
    double x, y;
    uint32 seed;
    
#pragma omp parallel private(seed) reduction (+:count_inside)
    {
        seed = 17 * omp_get_thread_num();
    
#pragma omp for private(x,y)
        for (int i = 0; i < points ; i++) {
            
            x = rand_r(&seed) / (double)RAND_MAX;
            y = rand_r(&seed) / (double)RAND_MAX;
            
            if( x * x + y * y < 1)  {
                count_inside++;
            }
        }
        
    }
    
    return (count_inside / points) * 4;
}

internal double
count_points_in_circle_threads(double const points) {
    double *array = new double[number_of_threads]();
    int32 current_thread, i, count_inside_thread = 0;
    double x,y;
    uint32 seed;
    int32 points_per_thread = points / number_of_threads;
    
#pragma omp parallel private (current_thread, x, y, i, seed, count_inside_thread) shared(array, points_per_thread) default(none)
    {
        count_inside_thread = 0;
        current_thread = omp_get_thread_num();
        seed = 17 * omp_get_thread_num();
    
        for (i = 0 ; i < points_per_thread ; i++){
            x = rand_r(&seed) / (double)RAND_MAX;
            y = rand_r(&seed) / (double)RAND_MAX;
            
            if( x * x + y * y < 1)  {
                count_inside_thread++;
            }
        }
        array[current_thread] = count_inside_thread;
    }
    
    double _ret_sum = 0.0;
    for (int32 i = 0; i < number_of_threads; ++i) {
        _ret_sum += array[i];
    }
    
    _ret_sum /= (int32)(points / number_of_threads) * number_of_threads;
    
    delete [] array;
    
    return _ret_sum * 4;
}

internal void
make_computation_and_print_elapsed_time(double (*function_to_call)(double), double argument) {
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC_RAW, &start);
    
    double computed_value = function_to_call(argument);
    
    clock_gettime(CLOCK_MONOTONIC_RAW, &end);
    
//    uint64_t delta_us = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_nsec - start.tv_nsec) / 1000;
    uint64_t delta_us = (end.tv_sec - start.tv_sec) * 1000 + (end.tv_nsec - start.tv_nsec) / 1000000;
    
    std::cout << "Computed value: " << computed_value << " in [ms] " << delta_us << std::endl;
}


internal void
init() {
#pragma omp parallel 
    {
        number_of_threads = omp_get_num_threads();
    }
    assert(number_of_threads > 0);
    std::cout << "Number of threads: " << number_of_threads << std::endl;
}

int main(int argc, const char * argv[]) {
    init();
    
    std::cout << "\nComputing intergals" << std::endl;
    
    double arg1 = 0.000000003;
    make_computation_and_print_elapsed_time(compute, arg1);
    make_computation_and_print_elapsed_time(compute_with_pragma_on_for, arg1);
    make_computation_and_print_elapsed_time(compute_with_array_without_private_subsum, arg1);
    make_computation_and_print_elapsed_time(compute_with_array_private_subsum, arg1);
    make_computation_and_print_elapsed_time(compute_with_array_private_subsum2, arg1);
    
    std::cout << "\nComputing Monte Carlo algorithm" << std::endl;
    
    double arg2 = 1000000000;
    make_computation_and_print_elapsed_time(count_points_in_circle_single_thread, arg2);
    make_computation_and_print_elapsed_time(count_points_in_circle_threads_on_loop, arg2);
    make_computation_and_print_elapsed_time(count_points_in_circle_threads, arg2);
    
    return 0;
}
