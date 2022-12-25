#include <iostream>
#include <vector>
#include <timsort.hpp>
#include <omp.h>
#include <math.h>
#include <algorithm>
#include <helper.hpp>
#include <string>
#include <chrono>

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        std::cout << "Input directory argument needed" << std::endl;
        return -1;
    }
    std::string filename = argv[1];
    int size = readDataCount(filename);
    int *data = readFile(filename);
    auto start = std::chrono::high_resolution_clock::now();
    TimSort<>::sort(data, data + size);
    auto end = std::chrono::high_resolution_clock::now();
    double duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    std::cout << "Seq," << size << "," << std::setprecision(9) << duration / 1000000000 << std::endl;
    return 0;
}
